// LIVELOCK: threads are busy all the time, but nobody makes progress.
//
// Same story as the deadlock: to sign you need BOTH the pen and the paper.
// This time both people are polite:
//   "If I can't get the second item, I put my first item back and try again."
// Alice and Bob move at exactly the same rhythm, so every round:
//   both take one item -> both see the other item is busy -> both put back
// ...and again, and again. Nobody is blocked, but nobody ever signs.

#include <chrono>
#include <cstdio>
#include <mutex>
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
    for (int attempt = 1; attempt <= 5; ++attempt) {
        first.m.lock();
        std::printf("%-5s: took the %s\n", who, first.name);
        std::this_thread::sleep_for(50ms);

        if (second.m.try_lock()) {  // try_lock never waits: it succeeds or fails right away
            std::printf("%-5s: took the %s too -> signed!\n", who, second.name);
            second.m.unlock();
            first.m.unlock();
            return;
        }

        std::printf("%-5s: the %s is busy -> put the %s back, try again\n", who, second.name, first.name);
        std::this_thread::sleep_for(50ms);  // putting it back takes a moment
        first.m.unlock();
        std::this_thread::sleep_for(50ms);  // BUG: both wait exactly the same time
    }
    std::printf("%-5s: gave up after 5 tries. Busy the whole time, signed nothing -> LIVELOCK.\n", who);
}

int main() {
    std::thread alice(sign, "Alice", std::ref(pen),   std::ref(paper));
    std::thread bob  (sign, "Bob",   std::ref(paper), std::ref(pen));
    alice.join();
    bob.join();
}
