#include <common.h>

#include <ogc/gx.h>
#include <bounding_region.h>
#include <chunk.h>

//#include <grrlib.h>

#include <ogc/cache.h>

#include <renderer.h>
#include <world.h>

//memalign:
#include <malloc.h> 

using namespace poyo;

#ifdef OPTIMIZATION_VECTOR
Chunk::Chunk(World* world) : cubitos_(CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE), region_(), world_(world)
{
    worldPosition_= {0, 0};
    offsetPosition_ = {0, 0};
    validBlocks_ = 0;
    validFaces_ = 0;
}
#else
Chunk::Chunk(World* world) : cubitos_(CHUNK_SIZE, Vector<Vector<Cubito>>(CHUNK_HEIGHT, Vector<Cubito>(CHUNK_SIZE))), region_(), world_(world) {
    worldPosition_= {0, 0};
    offsetPosition_ = {0, 0};
    validBlocks_ = 0;
    validFaces_ = 0;
}
#endif

Chunk::~Chunk() {
    if(displayListTransparent != nullptr) {
        DCFlushRange(displayListTransparent, displayListTransparentSize);
        free(displayListTransparent);
    }
    
    if(displayList != nullptr) {
        DCFlushRange(displayList, displayListSize);
        free(displayList);
    }
}


void Chunk::setCubito(const CubePosition& pos, BLOCK_TYPE block) {
#ifdef OPTIMIZATION_VECTOR
    //auto index = pos.x + CHUNK_SIZE * (pos.y + CHUNK_SIZE * pos.z); //better
    //auto index = (pos.y * CHUNK_SIZE * CHUNK_SIZE) + (pos.z * CHUNK_SIZE) + pos.x;
    auto index =  pos.x + CHUNK_SIZE * (pos.z + CHUNK_SIZE * pos.y);
    auto& currentCubito = cubitos_[index];
#else
    auto& currentCubito = cubitos_[pos.x][pos.y][pos.z];
#endif
    currentCubito.x = pos.x;
    currentCubito.y = pos.y;
    currentCubito.z = pos.z;
    currentCubito.type = block;

    if(hasProperty(block, FOLIAGE)) {
        fillCubito(currentCubito, 0, 0, 0, 0, DIR_DIAG_XY_FRONT, block);  // Front
        fillCubito(currentCubito, 1, 0, 0, 0, DIR_DIAG_XY_BACK,  block);  // Back
        fillCubito(currentCubito, 2, 0, 0, 0, DIR_Y_FRONT, BLOCK_AIR);  // Top
        fillCubito(currentCubito, 3, 0, 0, 0, DIR_Y_BACK,  BLOCK_AIR);  // Bottom
        fillCubito(currentCubito, 4, 0, 0, 0, DIR_Z_FRONT, BLOCK_AIR);  // Left
        fillCubito(currentCubito, 5, 0, 0, 0, DIR_Z_BACK,  BLOCK_AIR);  // Right
    }else {
        fillCubito(currentCubito, 0, 1, 0, 0, DIR_X_FRONT, block);  // Front
        fillCubito(currentCubito, 1, 0, 0, 0, DIR_X_BACK,  block);  // Back
        fillCubito(currentCubito, 2, 0, 1, 0, DIR_Y_FRONT, block);  // Top
        fillCubito(currentCubito, 3, 0, 0, 0, DIR_Y_BACK,  block);  // Bottom
        fillCubito(currentCubito, 4, 0, 0, 1, DIR_Z_FRONT, block);  // Left
        fillCubito(currentCubito, 5, 0, 0, 0, DIR_Z_BACK,  block);  // Right
    }
    
    if(block != BLOCK_AIR) {
        currentCubito.visible = true;
        validBlocks_++;
    }
}

static inline int round_up(int number, int multiple) {
    return ((number + multiple - 1) / multiple) * multiple;
}

void Chunk::createDisplayList() {
#ifdef OPTIMIZATION_OCCLUSION
#if OPTIMIZATION_OCCLUSION == 3 || OPTIMIZATION_OCCLUSION == 0
    U32 entitiesToRender = 0;
    //***** OPAQUES *****//
    Vector<Cubito> cubesOpaque;

    U16 validFaces = 0;
    for(auto& cube : cubitos_) {
        if(cube.visible && cube.type != BLOCK_WATER) {
            cubesOpaque.emplace_back(cube);
            for(auto& face : cube.face) {
                if(face.tile != NUM_TILES) validFaces++;
            }
        }
    }

    entitiesToRender = validFaces * 4;

    if(!cubesOpaque.empty()) {
        CreateList(entitiesToRender, displayList);
        Renderer::RenderCubeVector(cubesOpaque, entitiesToRender); //todo: fix this
        ListCreationDone(displayListSize);
    }

    //***** TRANSPARENTS *****//
    Vector<Cubito> cubesTransparent;

    validFaces = 0;
    for(auto& cube : cubitos_) {
        if(cube.visible && cube.type == BLOCK_WATER) {
            cubesTransparent.emplace_back(cube);
            for(auto& face : cube.face) {
                if(face.tile != NUM_TILES) validFaces++;
            }
        }
    }

    entitiesToRender = validFaces * 4;

    if(!cubesTransparent.empty()) {
        CreateList(entitiesToRender, displayListTransparent);
        Renderer::RenderCubeVectorIndexed(cubesTransparent, entitiesToRender);
        ListCreationDone(displayListTransparentSize);
    }
#elif OPTIMIZATION_OCCLUSION == 4
    U32 entitiesToRender = 0;
    //***** OPAQUES *****//
    Vector<Pair<CubeFace, USVec3>> facesOpaque;

    for(auto& cface : faces_) {
        if(cface.first.tile != TILE_WATER) facesOpaque.emplace_back(cface);
    }
    
    entitiesToRender = static_cast<U32>(facesOpaque.size()) * 4;
    
    if(!facesOpaque.empty()) {
        CreateList(entitiesToRender, displayList);
        Renderer::RenderFaceVector(facesOpaque, entitiesToRender);
        ListCreationDone(displayListSize);
    }
        
    //***** TRANSPARENTS *****//
    Vector<Pair<CubeFace, USVec3>> facesTransparent;
    for(auto& cface : faces_) {
        if(cface.first.tile == TILE_WATER) facesTransparent.emplace_back(cface);
    }
    
    entitiesToRender = static_cast<U32>(facesTransparent.size()) * 4;
    
    if(!facesTransparent.empty()) {
        CreateList(entitiesToRender, displayListTransparent);
        Renderer::RenderFaceVectorIndexed(facesTransparent, entitiesToRender);
        ListCreationDone(displayListTransparentSize);
    }
        
    //todo: Clear the Vector!!
    faces_.clear(); // Elimina los elementos pero no libera la capacidad
    Vector<Pair<CubeFace, USVec3>>().swap(faces_); // Libera toda la memoria consumida
#endif
    
#endif


#ifdef OPTIMIZATION_MODEL_MATRIX
    Renderer::CalculateModelMatrix(modelMatrix_, worldPosition_.x, 0, worldPosition_.z);
#endif
}

void Chunk::CreateList(U32 entitiesToRender, void*& list) {
    //The GX_DRAW_QUADS command takes up 3 bytes.
    //Each face is a quad with 4 vertexes.
    U32 VertexMemory = 0;
    //Each Vertex:
#ifdef OPTIMIZATION_VERTEX_MEMORY
    VertexMemory += 3 * sizeof(s16); // 3->s16 for positions
    #ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        VertexMemory += 3 * sizeof(s8);  // 3->s8 for normals
    #endif
    //VertexMemory += 4 * sizeof(u8);  // 4->u8 for colors
    VertexMemory += 2 * sizeof(u8);  // 2->u8 for texture coords
#else
    VertexMemory += 3 * sizeof(s16); // 3->s16 for positions
    #ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        VertexMemory += 3 * sizeof(s8);  // 3->s8 for normals
    #endif
    VertexMemory += 4 * sizeof(u8);  // 4->u8 for colors
    VertexMemory += 2 * sizeof(u16); //2->u16 for texture coords
#endif
    //Said by GX Documentation, an extra 63 bytes are needed.
    U32 listSize = 3 + entitiesToRender * VertexMemory + 63;
    //The list size also must be a multiple of 32, so round up to the next multiple of 32.
    //2 Options to Round Up:
    //listSize = round_up(listSize, 32);
    listSize = (listSize + 31) & ~31;   //Round up to a multiple of 32
    
    list = memalign(32, listSize);
    //Remove this block of memory from the CPU's cache because the write gather pipe is used to write the commands
    DCInvalidateRange(list, listSize);
    
    GX_BeginDispList(list, listSize);
}

void Chunk::ListCreationDone(U32& displaySize) const {
    displaySize = GX_EndDispList();
    assert(displayListSize != 0);
}

Blocks_Faces Chunk::occludeBlocks() {
    validBlocks_ = 0;
    validFaces_ = 0;
    
    for(auto& cubito : cubitos_) {
        if(!isVisible(cubito)) {
            cubito.visible = false;
            continue;
        }
        cubito.visible = !isCompletelyOccluded(cubito.x, cubito.y, cubito.z, offsetPosition_);
        if(cubito.visible) {
            validBlocks_++;
            for(auto& face : cubito.face) {
                if(face.tile != NUM_TILES) validFaces_++;
            }
        }
    }

    return Blocks_Faces(validBlocks_, validFaces_);
}

U32 Chunk::occludeBlocksFaces() {
    faces_.clear();
    for(auto& cubito : cubitos_) {
        if(cubito.visible) {
            if(hasProperty(cubito.type, FOLIAGE)) {
                faces_.emplace_back(cubito.face[0], USVec3(cubito.x, cubito.y, cubito.z));   // DIR_DIAG_XY_FRONT,        
                faces_.emplace_back(cubito.face[1], USVec3(cubito.x, cubito.y, cubito.z));   // DIR_DIAG_XY_BACK
            }else {
                if (!isSolid(cubito.x + 1, cubito.y, cubito.z, offsetPosition_)) {
                    if(cubito.type != BLOCK_WATER)
                    faces_.emplace_back(cubito.face[DIR_X_FRONT], USVec3(cubito.x, cubito.y, cubito.z));  // Front
                }
                if (!isSolid(cubito.x - 1, cubito.y, cubito.z, offsetPosition_)) {
                    if(cubito.type != BLOCK_WATER)
                    faces_.emplace_back(cubito.face[DIR_X_BACK], USVec3(cubito.x, cubito.y, cubito.z));   // Back
                }
                if (!isSolid(cubito.x, cubito.y + 1, cubito.z, offsetPosition_)) {
                    faces_.emplace_back(cubito.face[DIR_Y_FRONT], USVec3(cubito.x, cubito.y, cubito.z));  // Top
                }
                if (!isSolid(cubito.x, cubito.y - 1, cubito.z, offsetPosition_)) {
                    if(cubito.type != BLOCK_WATER)
                    faces_.emplace_back(cubito.face[DIR_Y_BACK], USVec3(cubito.x, cubito.y, cubito.z));   // Bottom
                }
                if (!isSolid(cubito.x, cubito.y, cubito.z + 1, offsetPosition_)) {
                    if(cubito.type != BLOCK_WATER)
                    faces_.emplace_back(cubito.face[DIR_Z_FRONT], USVec3(cubito.x, cubito.y, cubito.z));  // Left
                }
                if (!isSolid(cubito.x, cubito.y, cubito.z - 1, offsetPosition_)) {
                    if(cubito.type != BLOCK_WATER)
                    faces_.emplace_back(cubito.face[DIR_Z_BACK], USVec3(cubito.x, cubito.y, cubito.z));   // Right
                }
            }
        }
    }
    validFaces_ = static_cast<U16>(faces_.size());
    
    return 0;
}

void Chunk::updateVisibilityCount() {
    validFaces_ = 0;
    validBlocks_ = 0;
    for(auto& cube : cubitos_) {
        if(cube.visible) {
            validBlocks_++;
            for(auto& face : cube.face) {
                if(face.tile != NUM_TILES) validFaces_++;
            }
        }
    }
}

void Chunk::updateBoundingRegion() {
    FVec3 begin = FVec3(worldPosition_.x, 0, worldPosition_.z);
    FVec3 end = FVec3(worldPosition_.x + CHUNK_SIZE, CHUNK_HEIGHT, worldPosition_.z + CHUNK_SIZE);
    region_.setMinMax(begin, end);
}

void Chunk::render() {
    isBeingRendered = true;
#if !defined(OPTIMIZATION_DISPLAY_LIST)
    #ifdef OPTIMIZATION_OCCLUSION
        #if OPTIMIZATION_OCCLUSION == 0
            
        #elif OPTIMIZATION_OCCLUSION == 1
            occludeBlocks();
        #elif OPTIMIZATION_OCCLUSION == 2
            occludeBlocks();
            occludeBlocksFaces();
        #endif
        
    #endif
#endif

//NO OPTIMIZATIONS:
#if !defined(OPTIMIZATION_BATCHING) && !defined(OPTIMIZATION_DISPLAY_LIST)

#ifdef OPTIMIZATION_VECTOR
    for (const auto& cubito : cubitos_) {
        if(!cubito.visible) continue;
    #ifdef OPTIMIZATION_OCCLUSION
        #if OPTIMIZATION_OCCLUSION == 0
            Renderer::RenderCube(cubito, cFVec3(worldPosition_.x, 0, worldPosition_.z));
        #elif  OPTIMIZATION_OCCLUSION == 1
            Renderer::RenderCube(cubito, cFVec3(worldPosition_.x, 0, worldPosition_.z));
        #elif OPTIMIZATION_OCCLUSION == 2
            cFVec3 position = cFVec3(static_cast<float>(cubito.x) + static_cast<float>(worldPosition_.x),
                                     static_cast<float>(cubito.y) + 0,
                                     static_cast<float>(cubito.z) + static_cast<float>(worldPosition_.z));
            Renderer::ObjectView(position.x, position.y, position.z);

            for(auto& face : cubito.face) {
                if(face.tile != NUM_TILES) {
                    Renderer::RenderBegin(4);
                    Renderer::RenderFace(face);
                    Renderer::RenderEnd();
                }
            }
        #elif OPTIMIZATION_OCCLUSION == 3
            Renderer::RenderCube(cubito, cFVec3(worldPosition_.x, 0, worldPosition_.z));
        #elif OPTIMIZATION_OCCLUSION == 4
            cFVec3 position = cFVec3(static_cast<float>(cubito.x) + static_cast<float>(worldPosition_.x),
                         static_cast<float>(cubito.y) + 0,
                         static_cast<float>(cubito.z) + static_cast<float>(worldPosition_.z));
            Renderer::ObjectView(position.x, position.y, position.z);

            for(auto& face : cubito.face) {
                if(face.tile != NUM_TILES) {
                    Renderer::RenderBegin(4);
                    Renderer::RenderFace(face);
                    Renderer::RenderEnd();
                }
            }
        #endif
    #endif
    }
#else
    for (size_t x = 0; x < cubitos_.size(); ++x) {
            for (size_t y = 0; y < cubitos_[x].size(); ++y) {
                for (size_t z = 0; z < cubitos_[x][y].size(); ++z) {
                    const Cubito& currentCubito = cubitos_[x][y][z];
                    Renderer::RenderCube(currentCubito, cFVec3(worldPosition_.x, 0, worldPosition_.z));
                }
            }
        }
#endif
    
#endif

//OPTIMIZATION -> ONLY BATCHING
#if defined(OPTIMIZATION_BATCHING) && !defined(OPTIMIZATION_DISPLAY_LIST)

    #ifdef OPTIMIZATION_OCCLUSION
        #if OPTIMIZATION_OCCLUSION == 0
            Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
            Renderer::RenderCubeVector(cubitos_, validFaces_ * 4);
        #elif OPTIMIZATION_OCCLUSION == 1
            Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
            Renderer::RenderCubeVector(cubitos_, validFaces_ * 4);
        #elif OPTIMIZATION_OCCLUSION == 2
            Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
            Renderer::RenderFaceVector(faces_, validFaces_ * 4);
        #elif OPTIMIZATION_OCCLUSION == 3
            Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
            Renderer::RenderCubeVector(cubitos_, validFaces_ * 4);
        #elif OPTIMIZATION_OCCLUSION == 4
            Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
            Renderer::RenderFaceVector(faces_, validFaces_ * 4);
        #endif
    
    #endif

#endif

//OPTIMIZATION -> DISPLAY LIST + BATCHING
#if defined(OPTIMIZATION_BATCHING) && defined(OPTIMIZATION_DISPLAY_LIST)

    #ifdef OPTIMIZATION_MODEL_MATRIX
    // Mtx resultMatrix;
    // //Renderer::CalculateModelMatrix(modelMatrix_, worldPosition_.x, 0, worldPosition_.z); //3k ticks
    // guMtxConcat(Renderer::ViewMatrix(), modelMatrix_, resultMatrix);
    // GX_LoadPosMtxImm(resultMatrix, GX_PNMTX0);
    //todo: tengo un error que la iluminacion falla porque no calculo de nuevo la normal matrix... solo cuando hay iluminacion
        guMtxConcat(Renderer::ViewMatrix(), modelMatrix_, resultMatrix);
        GX_LoadPosMtxImm(resultMatrix, GX_PNMTX0);
    
    #else
        guMtxIdentity(resultMatrix);
        guMtxTransApply(resultMatrix, resultMatrix, worldPosition_.x, 0, worldPosition_.z);
        guMtxConcat(Renderer::ViewMatrix(), resultMatrix, resultMatrix);
        GX_LoadPosMtxImm(resultMatrix, GX_PNMTX0);
        //Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
    #endif
    
    Renderer::CallDisplayList(displayList, displayListSize);
#else

#endif

#ifdef OPTIMIZATION_DISPLAY_LIST
    Renderer::AddToFacesDrawn(validFaces_);
#endif
}

void Chunk::renderTranslucents() {
    if(displayListTransparent == nullptr) return;
    //OPTIMIZATION -> DISPLAY LIST + BATCHING
#if defined(OPTIMIZATION_BATCHING) && defined(OPTIMIZATION_DISPLAY_LIST)
    #ifdef OPTIMIZATION_MODEL_MATRIX
        GX_LoadPosMtxImm(resultMatrix, GX_PNMTX0);
    #else
    
        GX_LoadPosMtxImm(resultMatrix, GX_PNMTX0);
        //Renderer::ObjectView(worldPosition_.x, 0, worldPosition_.z);
    #endif
    Renderer::CallDisplayList(displayListTransparent, displayListTransparentSize);
#endif

}

bool Chunk::isVisible(const Cubito& cubito) const {
    return cubito.type != BLOCK_AIR;
}

bool Chunk::isSolid(const Cubito& cubito) const {
    //return cubito.type != BLOCK_AIR; //&& cubito.type != BLOCK_WATER; 
    //return cubito.type != BLOCK_AIR && cubito.type != BLOCK_DANDELION && cubito.type != BLOCK_WATER;
    return hasProperty(cubito.type, SOLID);
}

bool Chunk::isSolid(S16 x, S16 y, S16 z) const {
    //const auto index = x + CHUNK_SIZE * (y + CHUNK_SIZE * z); //better
    auto index =  x + CHUNK_SIZE * (z + CHUNK_SIZE * y);
    //return cubitos_[index].type != BLOCK_AIR; //&& cubitos_[index].type != BLOCK_WATER; 
    //return cubitos_[index].type != BLOCK_AIR && cubitos_[index].type != BLOCK_DANDELION && cubitos_[index].type != BLOCK_WATER; 
    return hasProperty(cubitos_[index].type, SOLID);
}

bool Chunk::isSolid(S16 x, S16 y, S16 z, const ChunkPosition& currentChunkPos) const {
    // Treat everything below the chunk as solid
    if (y < 0) return true;

    // Check if the position is outside the X or Z boundaries of the current chunk
    if (x < 0 || x >= CHUNK_SIZE || z < 0 || z >= CHUNK_SIZE) {
        ChunkPosition neighborChunkPos = currentChunkPos;
        
        // Adjust X coordinates and check neighbor chunk
        if (x < 0) {
            neighborChunkPos.x--; x += CHUNK_SIZE; // Look at the left chunk
        } else if (x >= CHUNK_SIZE) {
            neighborChunkPos.x++; x -= CHUNK_SIZE; // Look at the right chunk
        }

        // Adjust Z coordinates and check neighbor chunk
        if (z < 0) {
            neighborChunkPos.z--; z += CHUNK_SIZE; // Look at the back chunk
        } else if (z >= CHUNK_SIZE) {
            neighborChunkPos.z++; z -= CHUNK_SIZE; // Look at the front chunk
        }

        // Check if the neighbor chunk exists
        const Chunk* neighborChunk = world_->getChunk(neighborChunkPos.x, neighborChunkPos.z);
        if (neighborChunk) {
            // If exists, check if the cube in that neighbor chunk is solid
            return neighborChunk->isSolid(x, y, z);
        }

        // If no neighbor chunk, treat the position as non-solid
        return false;
    }

    // If the position is within the current chunk's bounds, just check this chunk
    return isSolid(x, y, z);
}

bool Chunk::isSameType(S16 x, S16 y, S16 z, U8 direction, BLOCK_TYPE type) const {
    switch (direction) {
    case DIR_X_FRONT:  // Frente en el eje X
        return World::isValidPosition(x + 1, y, z) && cubitos_[(x + 1) + CHUNK_SIZE * (y + CHUNK_SIZE * z)].type == type;
    case DIR_X_BACK:   // Detrás en el eje X
        return World::isValidPosition(x - 1, y, z) && cubitos_[(x - 1) + CHUNK_SIZE * (y + CHUNK_SIZE * z)].type == type;
    case DIR_Y_FRONT:  // Arriba en el eje Y
        return World::isValidPosition(x, y + 1, z) && cubitos_[x + CHUNK_SIZE * ((y + 1) + CHUNK_SIZE * z)].type == type;
    case DIR_Y_BACK:   // Abajo en el eje Y
        return World::isValidPosition(x, y - 1, z) && cubitos_[x + CHUNK_SIZE * ((y - 1) + CHUNK_SIZE * z)].type == type;
    case DIR_Z_FRONT:  // Izquierda en el eje Z
        return World::isValidPosition(x, y, z + 1) && cubitos_[x + CHUNK_SIZE * (y + CHUNK_SIZE * (z + 1))].type == type;
    case DIR_Z_BACK:   // Derecha en el eje Z
        return World::isValidPosition(x, y, z - 1) && cubitos_[x + CHUNK_SIZE * (y + CHUNK_SIZE * (z - 1))].type == type;
    default:
        return false;
    }
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
#if OPTIMIZATION_STRUCTS == 0
    currentFace.x = x;
    currentFace.y = y;
    currentFace.z = z;
#endif
    currentFace.direction = direction;
    if(direction != DIR_DIAG_XY_BACK && direction != DIR_DIAG_XY_FRONT) {
        currentFace.tile = blockTiles[block][direction];
    }else if(direction == DIR_DIAG_XY_FRONT) {
        currentFace.tile = blockTiles[block][0];
    }else if(direction == DIR_DIAG_XY_BACK) {
        currentFace.tile = blockTiles[block][1];
    }

    if(block != BLOCK_AIR) validFaces_++;
}



Cubito& Chunk::getCubito(const CubePosition& pos) {
#ifdef OPTIMIZATION_VECTOR
    //auto index = pos.x + CHUNK_SIZE * (pos.y + CHUNK_SIZE * pos.z);
    auto index =  pos.x + CHUNK_SIZE * (pos.z + CHUNK_SIZE * pos.y);
    return cubitos_[index];
#else
    return cubitos_[pos.x][pos.y][pos.z];
#endif
    
}


