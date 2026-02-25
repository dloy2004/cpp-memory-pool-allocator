#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "MemoryPoolThreadSafe.h"

struct Point { int x, y, z; };

int main() {
    const size_t blocks = 1000;
    MemoryPoolThreadSafe pool(sizeof(Point), blocks);
    std::cout << "Thread-safe pool initialized. capacity=" << pool.capacity() << "\n";

    auto worker = [&pool](int id) {
        for (int i = 0; i < 1000; ++i) {
            Point* p = static_cast<Point*>(pool.allocate());
            if (p) {
                p->x = id;
                p->y = i;
                pool.deallocate(p);
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    };

    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t) threads.emplace_back(worker, t);
    for (auto &th : threads) th.join();

    std::cout << "Work done. used_count=" << pool.used_count() << " (should be 0)\n";
    return 0;
}