#pragma once
#include <cstddef>
#include <vector>
#include <mutex>
#include <stdexcept>

class MemoryPoolThreadSafe {
private:
    struct FreeBlock {
        FreeBlock* next;
    };

    FreeBlock* freeListHead;
    void* memoryStart;
    size_t blockSize;
    size_t poolSize;
    size_t totalBytes;

    std::vector<char> allocationMap; // 0 = free, 1 = allocated (for validation / double-free detection)
    std::mutex mtx;

    static size_t align_up(size_t n, size_t align) {
        return (n + align - 1) & ~(align - 1);
    }

    inline ptrdiff_t block_index(void* ptr) const;

public:
    MemoryPoolThreadSafe(size_t sizeOfEachBlock, size_t numberOfBlocks);
    ~MemoryPoolThreadSafe();

    void* allocate();
    void deallocate(void* ptr);

    // diagnostics
    size_t capacity() const { return poolSize; }
    size_t used_count() const;
};