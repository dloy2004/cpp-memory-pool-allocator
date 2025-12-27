#pragma once
#include <cstddef> 
#include <vector>

class MemoryPool {
private:
    struct FreeBlock {
        FreeBlock* next;
    };

    FreeBlock* freeListHead;

    void* memoryStart;

    size_t blockSize;
    size_t poolSize;

public:
    MemoryPool(size_t sizeOfEachBlock, size_t numberOfBlocks);

    ~MemoryPool();

    void* allocate();

    void deallocate(void* ptr);
}; 
