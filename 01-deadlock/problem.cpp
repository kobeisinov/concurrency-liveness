// DEADLOCK: two threads wait for each other forever.
//
// Story: to sign a letter you need BOTH the pen and the paper.
//   Alice takes the pen first, then the paper.
//   Bob   takes the paper first, then the pen.
// Each one ends up holding one item and waiting for the other item.
// Nobody lets go, so nobody can ever continue.

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

    std::this_thread::sleep_for(100ms);  // give the other person time to grab their first item

    std::lock_guard<std::mutex> b(second.m);  // blocks forever: the other person holds it
    std::printf("%-5s: signed!\n", who);
    finished++;
}

int main() {
    std::thread(sign, "Alice", std::ref(pen),   std::ref(paper)).detach();
    std::thread(sign, "Bob",   std::ref(paper), std::ref(pen)).detach();  // BUG: opposite order

    std::this_thread::sleep_for(2s);
    if (finished == 2) std::printf("\nBoth signed. No deadlock.\n");
    else               std::printf("\nNobody finished after 2 seconds -> DEADLOCK.\n");
    std::fflush(stdout);
    std::_Exit(0);  // quit now, without waiting for the stuck threads
}
