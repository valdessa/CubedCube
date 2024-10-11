#include <typedefs.h>
#include <memory.h>

#include <malloc.h>  // For mallinfo function
#include <ogcsys.h>  // For SYS_GetArena1Lo and SYS_GetArena1Hi

using namespace poyo;

// Static member definition
static struct mallinfo info;  // Static structure to hold the mallinfo data

void Memory::updateInfo() {
    info = mallinfo();  // Updates mallinfo with the current memory status
}

int Memory::getTotalAllocatedMemory() {
    return info.uordblks;  // Returns total allocated memory
}

int Memory::getFreeMemory() {
    return info.fordblks;  // Returns total free memory
}

int Memory::getTotalMemory() {
    return info.arena;  // Returns the total size of the memory arena (heap)
}

int Memory::getNumberOfFreeChunks() {
    return info.ordblks;  // Returns the number of free chunks
}

int Memory::getNumberOfUsedChunks() {
    return info.uordblks;  // Returns the number of used chunks
}

size_t Memory::getTotalMemoryUsed() {
    updateInfo();
    return (uintptr_t)SYS_GetArena1Lo() - info.arena + info.uordblks - 0x80000000;
}

// struct mallinfo2 {
//     size_t arena;     /* Non-mmapped space allocated (bytes) */
//     size_t ordblks;   /* Number of free chunks */
//     size_t smblks;    /* Number of free fastbin blocks */
//     size_t hblks;     /* Number of mmapped regions */
//     size_t hblkhd;    /* Space allocated in mmapped regions
//                          (bytes) */
//     size_t usmblks;   /* See below */
//     size_t fsmblks;   /* Space in freed fastbin blocks (bytes) */
//     size_t uordblks;  /* Total allocated space (bytes) */
//     size_t fordblks;  /* Total free space (bytes) */
//     size_t keepcost;  /* Top-most, releasable space (bytes) */
// };