#ifndef INCLUDE_CHUNK_H_
#define INCLUDE_CHUNK_H_ 1

namespace poyo {
    class Chunk {
     public:
        Chunk(class World* world);
        ~Chunk();

        void setCubito(const CubePosition& pos, BLOCK_TYPE block);
        void createDisplayList();

        void render() const;
        void renderDisplayList() const;

        bool isSolid(S16 x, S16 y, S16 z) const;
        bool isSolid(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const;
        inline bool isCompletelyOccluded(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const {
            return isSolid(x + 1, y, z, currentChunkPos) &&  // Cubito at the right
                   isSolid(x - 1, y, z, currentChunkPos) &&  // Cubito at the left
                   isSolid(x, y + 1, z, currentChunkPos) &&  // Cubito at the top
                   isSolid(x, y - 1, z, currentChunkPos) &&  // Cubito at the bottom
                   isSolid(x, y, z + 1, currentChunkPos) &&  // Cubito at the front
                   isSolid(x, y, z - 1, currentChunkPos);    // Cubito at the back
        }
        
        Cubito& getCubito(const CubePosition& pos);

        U16 validBlocks = 0;
        ChunkPosition worldPosition_;
        ChunkPosition offsetPosition_;
        void* displayList = nullptr;
        U32 displayListSize = 0;
#ifdef OPTIMIZATION_VECTOR
        Vector<Cubito> cubitos_;
#else
        Vector<Vector<Vector<Cubito>>> cubitos_; //To Optimize
#endif
        
     private:
        World* world_ = nullptr; 
        void fillCubito(Cubito& cubito, U8 face, U8 x, U8 y, U8 z, U8 direction, S32 block);
    };
}

#endif