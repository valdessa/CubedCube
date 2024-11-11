#ifndef INCLUDE_WORLD_H_
#define INCLUDE_WORLD_H_ 1

#include "FastNoiseLite.h"

namespace poyo {
    class Chunk;
    class World {
     public:
        World();
        ~World();

        void generateLand(S16 Radius = 1);

        bool shouldPlaceTree(int localX, int baseY, int localZ, Chunk& chunk) const;
        void placeTree(Chunk& chunk, int x, int y, int z);

        int getGroundHeight(int worldX, int worldZ) const;

        static bool isValidPosition(S16 x, S16 y, S16 z) {
            return x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_SIZE;
        }

        static bool isValidPosition(const CubePosition& pos) {
            return pos.x >= 0 && pos.x < CHUNK_SIZE && pos.y >= 0 && pos.y < CHUNK_HEIGHT && pos.z >= 0 && pos.z < CHUNK_SIZE;
        }
        
        Chunk* getChunk(S16 chunkX, S16 chunkZ);
        const Vector<UPtr<Chunk>>& getChunks() const {
            return chunks_;
        }
        
        void occludeChunkBlocks();
        void occludeChunkBlocksFaces() const;
        
        void renderChunksAround(int playerX, int playerZ);
        
        U64 validBlocks_ = 0;
        U16 nTrees_ = 0;
     private:
        Vector<UPtr<Chunk>> chunks_;
        HashMap<Pair<S16, S16>, U16> positionMap_;

        FastNoiseLite noiseLite_;

        void generateLandChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);
        Chunk& getOrCreateChunkForLand(S16 chunkX, S16 chunkZ);
    };
}


#endif