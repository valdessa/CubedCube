#include <common.h>

#include <chunk.h>

#include <grrlib.h>
#include <ogc/gx.h>

#include "renderer.h"
#include <world.h>

using namespace poyo;

#ifdef OPTIMIZATION_VECTOR
Chunk::Chunk(World* world) : cubitos_(CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE), world_(world) {
    worldPosition_= {0, 0};
}
#else
Chunk::Chunk() : cubitos_(CHUNK_SIZE, Vector<Vector<Cubito>>(CHUNK_HEIGHT, Vector<Cubito>(CHUNK_SIZE))) {
    worldPosition_= {0, 0};
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

static inline int round_up(int number, int multiple) {
    return ((number + multiple - 1) / multiple) * multiple;
}

void Chunk::createDisplayList() {
    auto blockToRender = validBlocks * 4 * 6;
    //The GX_DRAW_QUADS command takes up 3 bytes.
    //Each face is a quad with 4 vertexes.
    //Each vertex takes up three u16 for the position coordinate, one u8 for the color index, and two u16 for the texture coordinate.
    //Because of the write gathering pipe, an extra 63 bytes are needed.
    size_t listSize = 3 + blockToRender * (3 * sizeof(s16) + 3 * sizeof(float) + 4 * sizeof(u8) + 2 * sizeof(u16)) + 63;
    //The list size also must be a multiple of 32, so round up to the next multiple of 32.
    listSize = round_up(listSize, 32);

    displayList = memalign(32, listSize);
    //Remove this block of memory from the CPU's cache because the write gather pipe is used to write the commands
    DCInvalidateRange(displayList, listSize);
    
    GX_BeginDispList(displayList, listSize); 
    Renderer::RenderCubeVector2(cubitos_, blockToRender);
    displayListSize = GX_EndDispList();
    assert(displayListSize != 0);
}

void Chunk::render() const {
#ifdef OPTIMIZATION_DISPLAY_LIST
    renderDisplayList();
#else
    u16 blockToRender = validBlocks * 24;
    Renderer::RenderCubeVector(cubitos_, blockToRender, cFVec3(worldPosition_.x, 0, worldPosition_.z));
#endif
}

void Chunk::renderDisplayList() const {
    GRRLIB_ObjectView(worldPosition_.x, 0, worldPosition_.z,
                  0.0f, 0.0f, 0.0f,
                  1.0f, 1.0f, 1.0f);
    GX_CallDispList(displayList, displayListSize);
}

bool Chunk::isSolid(const Cubito& cubito) const {
    return cubito.type != BLOCK_AIR; //&& cubito.type != BLOCK_WATER; 
}

bool Chunk::isSolid(S16 x, S16 y, S16 z) const {
    const auto index = x + CHUNK_SIZE * (y + CHUNK_SIZE * z); //better
    return cubitos_[index].type != BLOCK_AIR; //&& cubitos_[index].type != BLOCK_WATER; 
}

bool Chunk::isSolid(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const {
    // Si la posición está fuera de los límites del chunk actual
    if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        ChunkPosition neighborChunkPos = currentChunkPos;
        
        // Ajusta la posición del chunk vecino en función de la coordenada fuera de los límites  
        if (x < 0) {
            neighborChunkPos.x--; x += CHUNK_SIZE; // Mira el chunk a la izquierda
        } else if (x >= CHUNK_SIZE) {
            neighborChunkPos.x++; x -= CHUNK_SIZE; // Mira el chunk a la derecha
        }
        
        if (z < 0) {
            neighborChunkPos.z--; z += CHUNK_SIZE; // Mira el chunk de atrás
        } else if (z >= CHUNK_SIZE) {
            neighborChunkPos.z++; z -= CHUNK_SIZE; // Mira el chunk de adelante
        }

        // Verifica si el chunk vecino existe
        const Chunk* neighborChunk = world_->getChunk(neighborChunkPos.x, neighborChunkPos.z);
        if (neighborChunk) {
            // Si existe, revisa el cubo en el chunk vecino
            return neighborChunk->isSolid(x, y, z); // Verifica el bloque en el chunk vecino
        }
        return false; // Si no hay un chunk vecino, tratamos la posición como no sólida
    }
    // Si la posición está dentro del chunk actual, simplemente revisa el cubo en este chunk
    return isSolid(x, y, z);
}

bool Chunk::isCompletelyOccluded(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const {
    if (y == 0) {
        // Block at the bottom of the chunk: nothing below, only check sides and above
        return isSolid(x + 1, y, z, currentChunkPos) &&  // Block to the right
               isSolid(x - 1, y, z, currentChunkPos) &&  // Block to the left
               isSolid(x, y + 1, z, currentChunkPos) &&  // Block above
               isSolid(x, y, z + 1, currentChunkPos) &&  // Block in front
               isSolid(x, y, z - 1, currentChunkPos);    // Block behind
    } else if (y == CHUNK_HEIGHT - 1) {
        // Block at the top of the chunk: nothing above, check sides and below
        return isSolid(x + 1, y, z, currentChunkPos) &&  // Block to the right
               isSolid(x - 1, y, z, currentChunkPos) &&  // Block to the left
               isSolid(x, y - 1, z, currentChunkPos) &&  // Block below
               isSolid(x, y, z + 1, currentChunkPos) &&  // Block in front
               isSolid(x, y, z - 1, currentChunkPos);    // Block behind
    } else {
        // Block inside the chunk: check all sides, including above and below
        return isSolid(x + 1, y, z, currentChunkPos) &&  // Block to the right
               isSolid(x - 1, y, z, currentChunkPos) &&  // Block to the left
               isSolid(x, y + 1, z, currentChunkPos) &&  // Block above
               isSolid(x, y - 1, z, currentChunkPos) &&  // Block below
               isSolid(x, y, z + 1, currentChunkPos) &&  // Block in front
               isSolid(x, y, z - 1, currentChunkPos);    // Block behind
    }
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


