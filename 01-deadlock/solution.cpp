// DEADLOCK fixed: everybody takes the items in the SAME order.
//
// Same code as problem.cpp. Only one line in main() changed:
// Bob now takes the pen first, just like Alice.
// Whoever gets the pen first also gets the paper; the other one simply waits.

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
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
std::atomic<int> finished{0};

void sign(const char* who, Item& first, Item& second) {
    std::lock_guard<std::mutex> a(first.m);
    std::printf("%-5s: took the %s, waiting for the %s...\n", who, first.name, second.name);

    std::this_thread::sleep_for(100ms);

    std::lock_guard<std::mutex> b(second.m);
    std::printf("%-5s: signed!\n", who);
    finished++;
}

int main() {
    std::thread(sign, "Alice", std::ref(pen), std::ref(paper)).detach();
    std::thread(sign, "Bob",   std::ref(pen), std::ref(paper)).detach();  // FIX: same order as Alice

    std::this_thread::sleep_for(2s);
    if (finished == 2) std::printf("\nBoth signed. No deadlock.\n");
    else               std::printf("\nNobody finished after 2 seconds -> DEADLOCK.\n");
    std::fflush(stdout);
    std::_Exit(0);
}
