// STARVATION fixed: a fair lock that serves workers in the order they arrived.
//
// Same code as problem.cpp. Only the type of `printer` changed:
// std::mutex -> TicketLock.
//
// TicketLock works like the "take a number" machine at a bank:
//   lock():   take the next ticket number, wait until that number is called
//   unlock(): call the next number
// A worker who just finished gets a NEW ticket at the back of the line,
// so it cannot jump ahead of workers who were already waiting.

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

using namespace std::chrono_literals;

class TicketLock {
    std::atomic<int> next_ticket{0};
    std::atomic<int> now_serving{0};

public:
    void lock() {
        int my_ticket = next_ticket++;
        while (now_serving != my_ticket) std::this_thread::yield();
    }
    void unlock() { now_serving++; }
};

TicketLock printer;  // FIX: was std::mutex
std::atomic<bool> stop{false};
int turns[3] = {0, 0, 0};

void worker(int id) {
    while (!stop) {
        printer.lock();
        turns[id]++;
        std::this_thread::sleep_for(50ms);  // using the printer
        printer.unlock();
    }
}

int main() {
    std::thread w0(worker, 0), w1(worker, 1), w2(worker, 2);

    std::this_thread::sleep_for(2s);
    stop = true;
    w0.join(); w1.join(); w2.join();

    std::printf("Turns each worker got in 2 seconds (all 3 run the same code):\n");
    for (int id = 0; id < 3; ++id) std::printf("  worker %d: %2d turns\n", id, turns[id]);
}
