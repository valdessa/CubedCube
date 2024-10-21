#ifndef INCLUDE_CHUNK_H_
#define INCLUDE_CHUNK_H_ 1

namespace poyo {
    class Chunk {
     public:
        Chunk();
        ~Chunk();

        void setCubito(const CubePosition& pos, BLOCK_TYPE block);
        void createDisplayList();

        void render() const;
        void renderDisplayList() const;
        
        Cubito& getCubito(const CubePosition& pos);

        U16 validBlocks = 0;
        ChunkPosition position_;
        void* displayList = nullptr;
        U32 displayListSize = 0;
#ifdef OPTIMIZATION_VECTOR
        Vector<Cubito> cubitos_;
#else
        Vector<Vector<Vector<Cubito>>> cubitos_; //To Optimize
#endif
        
     private:

        void fillCubito(Cubito& cubito, U8 face, U8 x, U8 y, U8 z, U8 direction, S32 block);
    };
}

#endif