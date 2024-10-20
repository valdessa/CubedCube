#include <common.h>

#include <chunk.h>

using namespace poyo;

#ifdef OPTIMIZATION_VECTOR
Chunk::Chunk() : cubitos_(CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE) {
    
}
#else
Chunk::Chunk() : cubitos_(CHUNK_SIZE, Vector<Vector<Cubito>>(CHUNK_HEIGHT, Vector<Cubito>(CHUNK_SIZE))) {
    
}
#endif

Chunk::~Chunk() {
    
}

void Chunk::
setCubito(const CubePosition& pos, BLOCK_TYPE block) {
#ifdef OPTIMIZATION_VECTOR
    auto index = pos.x + CHUNK_SIZE * (pos.y + CHUNK_SIZE * pos.z); //better
    //auto index = (pos.y * CHUNK_SIZE * CHUNK_SIZE) + (pos.z * CHUNK_SIZE) + pos.x;
    auto& currentCubito = cubitos_[index];
#else
    auto& currentCubito = cubitos_[pos.x][pos.y][pos.z];
#endif
    currentCubito.x = pos.x;
    currentCubito.y = pos.y;
    currentCubito.z = pos.z;
    currentCubito.type = block;
    fillCubito(currentCubito, 0, 1, 0, 0, DIR_X_FRONT, block);  // Front
    fillCubito(currentCubito, 1, 0, 0, 0, DIR_X_BACK,  block);  // Back
    fillCubito(currentCubito, 2, 0, 1, 0, DIR_Y_FRONT, block);  // Top
    fillCubito(currentCubito, 3, 0, 0, 0, DIR_Y_BACK,  block);  // Bottom
    fillCubito(currentCubito, 4, 0, 0, 1, DIR_Z_FRONT, block);  // Left
    fillCubito(currentCubito, 5, 0, 0, 0, DIR_Z_BACK,  block);  // Right

    if(block != BLOCK_AIR) validBlocks++;
}

void Chunk::render() {
    
}

void Chunk::fillCubito(Cubito& cubito, U8 face, U8 x, U8 y, U8 z, U8 direction, S32 block) {
    auto& currentFace = cubito.face[face]; 
    currentFace.x = x;
    currentFace.y = y;
    currentFace.z = z;
    currentFace.direction = direction;
    currentFace.tile = blockTiles[block][direction];
}

Cubito& Chunk::getCubito(const CubePosition& pos) {
#ifdef OPTIMIZATION_VECTOR
    auto index = pos.x + CHUNK_SIZE * (pos.y + CHUNK_SIZE * pos.z);
    return cubitos_[index];
#else
    return cubitos_[pos.x][pos.y][pos.z];
#endif
    
}


