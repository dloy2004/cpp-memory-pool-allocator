#include "MemoryPool.h"
#include <iostream>
#include <cstdlib> 
using namespace std;

MemoryPool::MemoryPool(size_t sizeOfEachBlock, size_t numberOfBlocks) {
    this->blockSize = sizeOfEachBlock;
    this->poolSize = numberOfBlocks;

    size_t totalSize = sizeOfEachBlock * numberOfBlocks;
    this->memoryStart = malloc(totalSize);

    if (this->memoryStart == nullptr) {
        throw bad_alloc(); 
    }

    this->freeListHead = static_cast<FreeBlock*>(this->memoryStart);

    FreeBlock* currentBlock = this->freeListHead;

    for (size_t i = 0; i < numberOfBlocks - 1; ++i) {
        void* nextAddress = (char*)currentBlock + blockSize;

        currentBlock->next = static_cast<FreeBlock*>(nextAddress);

        currentBlock = currentBlock->next;
    }

    currentBlock->next = nullptr;
}

MemoryPool::~MemoryPool() {
    free(this->memoryStart);
}

void* MemoryPool::allocate() {
    if (this->freeListHead == nullptr) {
        return nullptr;
    }

    FreeBlock* blockToReturn = this->freeListHead;

    this->freeListHead = this->freeListHead->next;

    return blockToReturn;
}

void MemoryPool::deallocate(void* ptr) {
    if (ptr == nullptr) return;


    FreeBlock* blockToFree = static_cast<FreeBlock*>(ptr);

    blockToFree->next = this->freeListHead;

    this->freeListHead = blockToFree;
}