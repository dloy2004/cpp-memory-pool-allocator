#include <iostream>
#include "MemoryPool.h"
using namespace std;

struct Point {
    int x, y, z;
};

int main() {
    MemoryPool pool(sizeof(Point), 3);
    cout << "Pool initialized.\n";

    Point* p1 = (Point*)pool.allocate();
    p1->x = 10; p1->y = 20;
    cout << "Allocated p1 at address: " << p1 << "\n";

    Point* p2 = (Point*)pool.allocate();
    cout << "Allocated p2 at address: " << p2 << "\n";

    Point* p3 = (Point*)pool.allocate();
    cout << "Allocated p3 at address: " << p3 << "\n";

    Point* p4 = (Point*)pool.allocate();
    if (p4 == nullptr) {
        cout << "Pool is full correctly (p4 is null).\n";
    }

    pool.deallocate(p2);
    std::cout << "Deallocated p2.\n";

    Point* p5 = (Point*)pool.allocate();
    cout << "Allocated p5 at address: " << p5 << " (Should be same as p2)\n";

    return 0;
}