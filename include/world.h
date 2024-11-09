#ifndef INCLUDE_WORLD_H_
#define INCLUDE_WORLD_H_ 1

#include "noise.h"
#include "FastNoiseLite.h"

namespace poyo {
    class Chunk;
    class World {
     public:
        World();
        ~World();

        void generateLand(S16 Radius = 1);
        void generateLandChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);
        Chunk& getOrCreateChunkForLand(S16 chunkX, S16 chunkZ);

        bool shouldPlaceTree(int localX, int localY, int localZ, Chunk& chunk) const;
        void placeTree(Chunk& chunk, int x, int y, int z);

        int getGroundHeight(int worldX, int worldZ, const Chunk& chunk) const;

        bool isValidPosition(const CubePosition& pos) const {
            return pos.x >= 0 && pos.x < CHUNK_SIZE && pos.y >= 0 && pos.y < CHUNK_HEIGHT && pos.z >= 0 && pos.z < CHUNK_SIZE;
        }

        
        void generateChunks(S16 middleX, S16 middleZ, S16 numChunksX, S16 numChunksZ);

        void generateChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);
        void generateSolidChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);

        Chunk& getOrCreateChunk(S16 chunkX, S16 chunkZ);
        Chunk* getChunk(S16 chunkX, S16 chunkZ);

        const Vector<UPtr<Chunk>>& getChunks() const {
            return chunks_;
        }
        
        void occludeChunkBlocks();
        void occludeChunkBlocksFaces() const;

        
        void renderChunksAround(int playerX, int playerZ);
        
        FastNoiseLite noiseLite_;
        Noise noise_;
        
        U32 helperCounter = 0;
        U64 validBlocks_ = 0;
        U16 nTrees_ = 0;
     private:
        Vector<UPtr<Chunk>> chunks_;
        HashMap<Pair<S16, S16>, U16> positionMap_;

        float getHeightAt(S32 x, S32 z);
    };
}


#endif