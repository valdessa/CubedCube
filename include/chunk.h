#ifndef INCLUDE_CHUNK_H_
#define INCLUDE_CHUNK_H_ 1

namespace poyo {
    class Chunk {
     public:
        Chunk(class World* world);
        ~Chunk();

        void setCubito(const CubePosition& pos, BLOCK_TYPE block);
        void createDisplayList();

        U32 occludeBlocks();
        U32 occludeBlocksFaces();

        void render() const;

        bool isSolid(const Cubito& cubito) const;
        bool isSolid(S16 x, S16 y, S16 z) const;
        bool isSolid(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const;
        bool isCompletelyOccluded(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const;
        
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
        
    Vector<Pair<CubeFace, USVec3>> faces_;
        
     private:
        World* world_ = nullptr; 
        void fillCubito(Cubito& cubito, U8 face, U8 x, U8 y, U8 z, U8 direction, S32 block);
    };
}

#endif