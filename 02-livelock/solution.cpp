// LIVELOCK fixed: after a failed try, wait a RANDOM time before trying again.
//
// Same code as problem.cpp. Only the last wait changed.
// Because Alice and Bob now wait different amounts of time, they fall out of
// step. Soon one of them grabs both items while the other one is waiting.

#include <chrono>
#include <cstdio>
#include <mutex>
#include <random>
#include <thread>

using namespace std::chrono_literals;

struct Item {
    const char* name;
    std::mutex  m;
    Item(const char* n) : name(n) {}
};

Item pen{"pen"};
Item paper{"paper"};

void sign(const char* who, Item& first, Item& second) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> random_ms(0, 200);

    for (int attempt = 1; ; ++attempt) {
        first.m.lock();
        std::printf("%-5s: took the %s\n", who, first.name);
        std::this_thread::sleep_for(50ms);

        if (second.m.try_lock()) {
            std::printf("%-5s: took the %s too -> signed! (attempt %d)\n", who, second.name, attempt);
            second.m.unlock();
            first.m.unlock();
            return;
        }

        std::printf("%-5s: the %s is busy -> put the %s back, try again\n", who, second.name, first.name);
        std::this_thread::sleep_for(50ms);
        first.m.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(random_ms(rng)));  // FIX: random wait
    }
}

int main() {
    std::thread alice(sign, "Alice", std::ref(pen),   std::ref(paper));
    std::thread bob  (sign, "Bob",   std::ref(paper), std::ref(pen));
    alice.join();
    bob.join();
}
