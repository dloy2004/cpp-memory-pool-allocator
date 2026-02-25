#include "MemoryPoolThreadSafe.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cassert>

MemoryPoolThreadSafe::MemoryPoolThreadSafe(size_t sizeOfEachBlock, size_t numberOfBlocks) {
    // align block size to at least pointer size and ensure it can store FreeBlock
    size_t minBlock = std::max(sizeof(FreeBlock), sizeof(void*));
    const size_t alignment = alignof(std::max_align_t);
    this->blockSize = align_up(std::max(sizeOfEachBlock, minBlock), alignment);
    this->poolSize = numberOfBlocks;
    this->totalBytes = this->blockSize * this->poolSize;

    this->memoryStart = std::malloc(this->totalBytes);
    if (!this->memoryStart) throw std::bad_alloc();

    // init free list
    this->freeListHead = static_cast<FreeBlock*>(this->memoryStart);
    FreeBlock* cur = this->freeListHead;
    for (size_t i = 0; i < numberOfBlocks - 1; ++i) {
        void* nextAddr = (char*)cur + this->blockSize;
        cur->next = static_cast<FreeBlock*>(nextAddr);
        cur = cur->next;
    }
    cur->next = nullptr;

    allocationMap.assign(this->poolSize, 0);
}

MemoryPoolThreadSafe::~MemoryPoolThreadSafe() {
    std::free(this->memoryStart);
}

inline ptrdiff_t MemoryPoolThreadSafe::block_index(void* ptr) const {
    if (!ptr) return -1;
    ptrdiff_t offset = static_cast<char*>(ptr) - static_cast<char*>(this->memoryStart);
    if (offset < 0 || static_cast<size_t>(offset) >= totalBytes) return -1;
    if (offset % static_cast<ptrdiff_t>(blockSize) != 0) return -1;
    return offset / static_cast<ptrdiff_t>(blockSize);
}

void* MemoryPoolThreadSafe::allocate() {
    std::lock_guard<std::mutex> lk(mtx);
    if (this->freeListHead == nullptr) return nullptr;
    FreeBlock* ret = this->freeListHead;
    this->freeListHead = this->freeListHead->next;

    ptrdiff_t idx = block_index(ret);
    assert(idx >= 0 && static_cast<size_t>(idx) < poolSize);
    allocationMap[static_cast<size_t>(idx)] = 1;
    return ret;
}

void MemoryPoolThreadSafe::deallocate(void* ptr) {
    if (ptr == nullptr) return;

    std::lock_guard<std::mutex> lk(mtx);

    ptrdiff_t idx = block_index(ptr);
    if (idx < 0) {
        // pointer not from this pool
        std::cerr << "Error: deallocate() - pointer not in pool\n";
        return;
    }

    if (allocationMap[static_cast<size_t>(idx)] == 0) {
        // double-free or corruption
        std::cerr << "Error: deallocate() - double-free or invalid free detected (index=" << idx << ")\n";
        return;
    }

    allocationMap[static_cast<size_t>(idx)] = 0;

    FreeBlock* block = static_cast<FreeBlock*>(ptr);
    block->next = this->freeListHead;
    this->freeListHead = block;
}

size_t MemoryPoolThreadSafe::used_count() const {
    std::lock_guard<std::mutex> lk(const_cast<std::mutex&>(mtx));
    size_t cnt = 0;
    for (char c : allocationMap) if (c) ++cnt;
    return cnt;
}