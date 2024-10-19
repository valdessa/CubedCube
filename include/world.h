#ifndef INCLUDE_WORLD_H_
#define INCLUDE_WORLD_H_ 1

#include "noise.h"

namespace poyo {
    class Chunk;
    class World {
     public:
        World();
        ~World();

        void generateChunks(S16 startX, S16 startZ, S16 numChunksX, S16 numChunksZ);

        void generateChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);
        void generateSolidChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);

        Chunk& getOrCreateChunk(S16 chunkX, S16 chunkZ);
        const HashMap<Pair<S16, S16>, Chunk>& getChunks() const {
            return chunks_;
        }

        Noise noise_;
        
        U32 helperCounter = 0;
        U64 validBlocks_ = 0;
     private:
        HashMap<Pair<S16, S16>, Chunk> chunks_;

        float getHeightAt(S32 x, S32 z);
    };
}


#endif