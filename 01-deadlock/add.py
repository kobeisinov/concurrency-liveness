import numpy as np
from numba import cuda, types as numba_types
from time import perf_counter


# 1. DEVICE FUNCTION
@cuda.jit(device=True)
def device_add(a, b):
    return a + b


# 2. GPU KERNEL WITH GRID-STRIDE LOOP
@cuda.jit
def add_arrays(a, b, c):
    i = cuda.grid(1)
    stride = cuda.gridsize(1)

    while i < c.size:
        c[i] = device_add(a[i], b[i])
        i += stride


# 3. REDUCTION4: FIRST ADD DURING LOAD
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


# 4. MULTI-PASS REDUCTION
def gpu_sum(d_array):
    TPB = 256
    current = d_array

    while current.size > 1:
        blocks = (current.size + 2 * TPB - 1) // (2 * TPB)

        d_out = cuda.device_array(blocks, dtype=np.int32)

        shared_bytes = TPB * np.dtype(np.int32).itemsize

        reduce4[blocks, TPB, 0, shared_bytes](
            current, d_out
        )

        current = d_out

    return int(current.copy_to_host()[0])


# 5. MAIN PROGRAM
def main(n):
    TPB = 256
    blocks = (n + TPB - 1) // TPB

    # CPU arrays
    a = (np.arange(n, dtype=np.int32) % 11) - 5
    b = ((np.arange(n, dtype=np.int32) * 3) % 11) - 5

    # Streams
    stream1 = cuda.stream()
    stream2 = cuda.stream()

    # CPU -> GPU
    d_a = cuda.to_device(a, stream=stream1)
    d_b = cuda.to_device(b, stream=stream2)

    stream1.synchronize()
    stream2.synchronize()

    # Allocate GPU output
    d_c = cuda.device_array(n, dtype=np.int32)

    # Run GPU kernel
    add_arrays[blocks, TPB](d_a, d_b, d_c)

    # GPU -> CPU
    result = d_c.copy_to_host()

    # Verify results
    expected = a + b

    print("Arrays correct:", np.array_equal(result, expected))
    print("GPU sum:", gpu_sum(d_c))
    print("CPU sum:", np.sum(expected, dtype=np.int64))


if __name__ == "__main__":
    main(10)
    main(2 << 20)
