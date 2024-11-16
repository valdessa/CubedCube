#ifndef INCLUDE_CHUNK_H_
#define INCLUDE_CHUNK_H_ 1

namespace poyo {
    class Chunk {
     public:
        Chunk(class World* world);
        ~Chunk();

        void setCubito(const CubePosition& pos, BLOCK_TYPE block);
        void createDisplayList();

        Blocks_Faces occludeBlocks();
        U32 occludeBlocksFaces();

        void updateVisibilityCount();

        void render();
        void renderTranslucents();

        bool isVisible(const Cubito& cubito) const;
        bool isSolid(const Cubito& cubito) const;
        bool isSolid(S16 x, S16 y, S16 z) const;
        bool isSolid(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const;

        bool isSameType(S16 x, S16 y, S16 z, U8 direction, BLOCK_TYPE type) const;
        
        bool isCompletelyOccluded(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const;
        
        Cubito& getCubito(const CubePosition& pos);
            
        U16 validBlocks_;
        U16 validFaces_;
            
        ChunkPosition worldPosition_;
        ChunkPosition offsetPosition_;
        void* displayList = nullptr;
        void* displayListTransparent = nullptr;
        U32 displayListSize = 0;
        U32 displayListTransparentSize = 0;
#ifdef OPTIMIZATION_VECTOR
        Vector<Cubito> cubitos_;
#else
        Vector<Vector<Vector<Cubito>>> cubitos_; //To Optimize
#endif

#ifdef OPTIMIZATION_MODEL_MATRIX
        Mtx modelMatrix_;
#endif
        Mtx resultMatrix;
        
        Vector<Pair<CubeFace, USVec3>> faces_;
        
     private:
        World* world_;
            
        void fillCubito(Cubito& cubito, U8 face, U8 x, U8 y, U8 z, U8 direction, S32 block);

        void CreateList(U32 entitiesToRender, void*& list);
        void ListCreationDone(U32& displaySize) const;
    };
}

#endif