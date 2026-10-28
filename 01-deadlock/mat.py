import numpy as np
from numba import cuda, types as numba_types
from time import perf_counter


@cuda.jit(device=True)
def square(x):
    return x * x


@cuda.jit(device=True)
def double(x):
    return 2 * x


@cuda.jit
def transform(a, b, c):
    col, row = cuda.grid(2)
    stride_col, stride_row = cuda.gridsize(2)

    rows, cols = a.shape

    for r in range(row, rows, stride_row):
        for j in range(col, cols, stride_col):
            idx = r * cols + j
            c[idx] = square(a[r, j]) + double(b[r, j])


@cuda.jit
def reduce4(g_idata, g_odata):
    sdata = cuda.shared.array(
        shape=0, dtype=numba_types.int32
    )

    tid = cuda.threadIdx.x
    block_size = cuda.blockDim.x

    i = cuda.blockIdx.x * (2 * block_size) + tid

    total = 0

    if i < g_idata.size:
        total += g_idata[i]

    if i + block_size < g_idata.size:
        total += g_idata[i + block_size]

    sdata[tid] = total
    cuda.syncthreads()

    s = block_size // 2

    while s > 0:
        if tid < s:
            sdata[tid] += sdata[tid + s]

        cuda.syncthreads()
        s //= 2

    if tid == 0:
        g_odata[cuda.blockIdx.x] = sdata[0]


def gpu_sum(d_array):
    TPB = 256
    current = d_array

    while current.size > 1:
        blocks = (current.size + 2 * TPB - 1) // (2 * TPB)

        d_next = cuda.device_array(blocks, dtype=np.int32)

        shared_bytes = TPB * np.dtype(np.int32).itemsize

        reduce4[blocks, TPB, 0, shared_bytes](
            current, d_next
        )

        current = d_next

    return int(current.copy_to_host()[0])


def main(n):
    values = np.arange(n * n, dtype=np.int32)

    a = (values.reshape(n, n) % 11) - 5
    b = ((values.reshape(n, n) * 3) % 11) - 5

    # CPU baseline
    start = perf_counter()
    expected = a * a + 2 * b
    cpu_time = perf_counter() - start

    # Streams and CPU -> GPU
    s1 = cuda.stream()
    s2 = cuda.stream()

    d_a = cuda.to_device(a, stream=s1)
    d_b = cuda.to_device(b, stream=s2)

    s1.synchronize()
    s2.synchronize()

    # Output is flattened for easier reduction
    d_c = cuda.device_array(n * n, dtype=np.int32)

    threads = (16, 16)
    blocks = (
        (n + 15) // 16,
        (n + 15) // 16
    )

    # GPU computation
    start = perf_counter()

    transform[blocks, threads](d_a, d_b, d_c)
    cuda.synchronize()

    gpu_time = perf_counter() - start

    # GPU -> CPU
    result = d_c.copy_to_host().reshape(n, n)

    # Verification
    print("N =", n)
    print("Correct:", np.array_equal(result, expected))
    print("CPU sum:", np.sum(expected, dtype=np.int64))
    print("GPU sum:", gpu_sum(d_c))
    print("CPU time:", cpu_time)
    print("GPU time:", gpu_time)

    if n == 4:
        print("Result:\n", result)

    del d_a, d_b, d_c


if __name__ == "__main__":
    main(4)
    main(1024)
