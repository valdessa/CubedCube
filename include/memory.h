#ifndef INCLUDE_MEMORY_H_
#define INCLUDE_MEMORY_H_ 1

namespace poyo {
    class Memory {
     public:
        // Updates the mallinfo data
        static void updateInfo();

        // Getters for the relevant fields
        static int getTotalAllocatedMemory();
        static int getFreeMemory();
        static int getTotalMemory();
        static int getNumberOfFreeChunks();
        static int getNumberOfUsedChunks();

        // Custom method to calculate total memory used, including arenas
        static size_t getTotalMemoryUsed();
        static size_t getTotalMemoryFree();
    };
}

#endif
