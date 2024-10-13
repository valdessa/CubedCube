#ifndef INCLUDE_WORLD_H_
#define INCLUDE_WORLD_H_ 1

namespace poyo {
    class Chunk;
    class World {
     public:
        World();
        ~World();

        void generateChunk(Chunk& chunk, S16 chunkX, S16 chunkZ);

        Chunk& getOrCreateChunk(S16 chunkX, S16 chunkZ);
        U32 helperCounter = 0;
     private:
        HashMap<Pair<S16, S16>, Chunk> chunks_;

        float getHeightAt(S32 x, S32 z);
    };
}


#endif