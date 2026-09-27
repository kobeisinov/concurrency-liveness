// STARVATION: some threads almost never get their turn.
//
// Story: 3 workers share ONE printer (a std::mutex).
// Each worker does the same thing in a loop:
//   lock the printer -> print for 50 ms -> unlock -> immediately get back in line
//
// std::mutex does NOT promise fairness. When a worker unlocks, the others
// are asleep and need time to wake up. The worker who just unlocked is
// already running, so it grabs the lock again first. Again and again.
// The other workers are "starving": they wait, but their turn never comes.

#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;

std::mutex printer;
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
