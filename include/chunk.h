#ifndef INCLUDE_CHUNK_H_
#define INCLUDE_CHUNK_H_ 1
#include "common.h"

namespace poyo {
    
    class Chunk {
     public:
        Chunk();
        ~Chunk();

        void setCubito(const CubePosition& pos, BLOCK_TYPE block);

        void render();
        
        Cubito& getCubito(const CubePosition& pos);

        U32 validBlocks = 0;
        
#ifdef OPTIMIZE_VECTOR
        Vector<Cubito> cubitos_;
#else
        Vector<Vector<Vector<Cubito>>> cubitos_; //To Optimize
#endif
        
     private:

        void fillCubito(Cubito& cubito, U8 face, U8 x, U8 y, U8 z, U8 direction, S32 block);
    };
}

#endif