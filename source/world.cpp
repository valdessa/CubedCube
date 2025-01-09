#include <common.h>
#include <ogc/cache.h>

namespace std {
    // Custom hash function for std::pair<short, short>
    template<>
    struct hash<std::pair<short, short>> {
        // Combines the hashes of both elements in the pair
        std::size_t operator()(const std::pair<short, short>& p) const noexcept {
            // Hash each element of the pair
            std::size_t h1 = std::hash<short>{}(p.first);
            std::size_t h2 = std::hash<short>{}(p.second);

            // Combine the two hashes using XOR and bit-shifting
            return h1 ^ (h2 << 1); // Combined hash value
        }
    };
}
#include <world.h>

#include <ogc/gx.h> //for mtx
#include <chunk.h>

#include <renderer.h>

using namespace poyo;

World::World() {
#ifdef KIRBY_EASTER_EGG
    NKirbys = 0;
    kirbyTransforms_.reserve(MAX_KIRBY);
#endif
    //noiseLite_.SetSeed(seed);
    noiseLite_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noiseLite_.SetFrequency(0.05f);
}

World::~World() {
    //Delete all display lists 
    for(auto& chunk : chunks_) {
        chunk.reset(); 
    }
}

void World::generateLand(S16 radius) {
    for (int chunkX = -radius; chunkX <= radius; ++chunkX) {
        for (int chunkZ = -radius; chunkZ <= radius; ++chunkZ) {
            const auto& currentChunk = getOrCreateChunkForLand(
                static_cast<S16>(chunkX),
                static_cast<S16>(chunkZ));
            validBlocks_ += currentChunk.validBlocks_;
            validFaces_  += currentChunk.validFaces_;
        }
    }

#ifdef OPTIMIZATION_OCCLUSION
    #if OPTIMIZATION_OCCLUSION == 3
        occludeChunkBlocks();
    #elif OPTIMIZATION_OCCLUSION == 4
        occludeChunkBlocks();
        occludeChunkBlocksFaces();
    #endif
#endif

#ifdef OPTIMIZATION_DISPLAY_LIST
    for(auto& chunks : getChunks()) {
        chunks->createDisplayList();
    }
#endif
}

static void placeWater(Chunk& chunk, const CubePosition& pos) {
    // bool CanBePlaced = true;
    //
    // auto positionToCheck = CubePosition(pos.x, pos.y - 1, pos.z);
    // if(World::isValidPosition(positionToCheck)) {
    //     if(chunk.getCubito(positionToCheck).type == BLOCK_WATER) CanBePlaced = false;
    // }
    //
    // if(CanBePlaced) chunk.setCubito(pos, BLOCK_WATER);
}

void World::generateLandChunk(Chunk& chunk, S16 chunkX, S16 chunkZ) {
    // Set the chunk's offset and world position
    chunk.offsetPosition_.x = chunkX;
    chunk.offsetPosition_.z = chunkZ;
    chunk.worldPosition_.x = static_cast<S16>(chunkX * CHUNK_SIZE);
    chunk.worldPosition_.z = static_cast<S16>(chunkZ * CHUNK_SIZE);
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Calculate the global position in the world
            int worldX = x + (chunkX * CHUNK_SIZE);
            int worldZ = z + (chunkZ * CHUNK_SIZE);

            // Generate a height based on noise for the position (x, z)
            float height = noiseLite_.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
            int blockHeight = static_cast<int>((height + 1) * (CHUNK_HEIGHT / 2));

            // Assign blocks based on the generated height
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                CubePosition pos(x, y, z);
                if (y > blockHeight) {
                    if (y == WATER_LEVEL) chunk.setCubito(pos, BLOCK_WATER);  // Fill any space below WATER_LEVEL with water
                    else                  chunk.setCubito(pos, BLOCK_AIR);    // Air above water
                } else if (y == blockHeight) { //The Top of the Height Map
                    //if (y > WATER_LEVEL) chunk.setCubito(pos, BLOCK_GRASS);  
                    if(y == WATER_LEVEL)     chunk.setCubito(pos, BLOCK_WATER);
                    else if(y < WATER_LEVEL) chunk.setCubito(pos, BLOCK_AIR);
                    else                     chunk.setCubito(pos, BLOCK_GRASS);  // Grass at the top
                } else if (y > blockHeight - DIRT_LEVEL) {
                    chunk.setCubito(pos, BLOCK_DIRT);   // Dirt below the grass
                } else if (y > blockHeight - STONE_LEVEL) {
                    chunk.setCubito(pos, BLOCK_STONE);  // Stone below the dirt
                } else {
                    chunk.setCubito(pos, BLOCK_STONE); // Todo: all the blocks
                }
            }
        }
    }

    for(int i = 0; i < MAX_FLOWERS; i++) {
        // Generate a random position within the chunk
        int x = rand() % CHUNK_SIZE;
        int z = rand() % CHUNK_SIZE;

        // Calculate the global position in the world
        int worldX = x + (chunkX * CHUNK_SIZE);
        int worldZ = z + (chunkZ * CHUNK_SIZE);
        // Get the terrain height at (worldX, worldZ)
        int randY = getGroundHeight(worldX, worldZ);

        if(chunk.getCubito(CubePosition(x, randY, z)).type == BLOCK_GRASS) {
            if(isValidPosition(CubePosition(x, randY + 1, z))) {
                if(chunk.getCubito(CubePosition(x, randY + 1, z)).type == BLOCK_AIR) {
                    int randN = rand() % 3;
                    switch (randN) {
                        case 0: chunk.setCubito(CubePosition(x, randY + 1, z), BLOCK_POPPY);
                            break;
                        case 1: chunk.setCubito(CubePosition(x, randY + 1, z), BLOCK_ORCHID);
                            break;
                        case 2: chunk.setCubito(CubePosition(x, randY + 1, z), BLOCK_DANDELION);
                            break;
                    }
                }
            }
        }
    }

    for(int i = 0; i < MAX_HERBS; i++) {
        // Generate a random position within the chunk
        int x = rand() % CHUNK_SIZE;
        int z = rand() % CHUNK_SIZE;

        // Calculate the global position in the world
        int worldX = x + (chunkX * CHUNK_SIZE);
        int worldZ = z + (chunkZ * CHUNK_SIZE);
        // Get the terrain height at (worldX, worldZ)
        int randY = getGroundHeight(worldX, worldZ);

        if(chunk.getCubito(CubePosition(x, randY, z)).type == BLOCK_GRASS) {
            if(isValidPosition(CubePosition(x, randY + 1, z))) {
                if(chunk.getCubito(CubePosition(x, randY + 1, z)).type == BLOCK_AIR) {
                    chunk.setCubito(CubePosition(x, randY + 1, z), BLOCK_HERB);
                }
            }
        }
    }


    for(int i = 0; i < MAX_TREES; i++) {
        // Generate a random position within the chunk
        int x = rand() % CHUNK_SIZE;
        int z = rand() % CHUNK_SIZE;

        // Calculate the global position in the world
        int worldX = x + (chunkX * CHUNK_SIZE);
        int worldZ = z + (chunkZ * CHUNK_SIZE);
        // Get the terrain height at (worldX, worldZ)
        int randY = getGroundHeight(worldX, worldZ); 

        // Check if we can place a tree at this position
        if (shouldPlaceTree(x, randY + 1, z, chunk)) {
            // Place the tree if possible
            placeTree(chunk, x, randY + 1, z);
        }
    }

#ifdef KIRBY_EASTER_EGG
    if(NKirbys < MAX_KIRBY) {
        // Generate a random position within the chunk
        int x = rand() % CHUNK_SIZE;
        int z = rand() % CHUNK_SIZE;

        // Calculate the global position in the world
        int worldX = x + (chunkX * CHUNK_SIZE);
        int worldZ = z + (chunkZ * CHUNK_SIZE);
        // Get the terrain height at (worldX, worldZ)
        int randY = getGroundHeight(worldX, worldZ);

        // Check if we can place a kirby at this position
        if (shouldPlaceKirby(x, randY + 1, z, chunk)) {
            // Place the tree if possible
            float kirbyPosX = x + chunk.worldPosition_.x;
            float kirbyPosZ = z + chunk.worldPosition_.z;
            auto& result = kirbyTransforms_.emplace_back(Transform(FVec3{kirbyPosX + 0.5f, randY + 1, kirbyPosZ + 0.5f}, FVec3{0, rand()%360, 0}, FVec3{0.005f}));
            Renderer::CalculateModelMatrix(result.modelMatrix, result.transform);

            NKirbys++;
        }
    }
#endif

    chunk.updateVisibilityCount();
}

Chunk& World::getOrCreateChunkForLand(S16 chunkX, S16 chunkZ) {
    const auto chunkKey = std::make_pair(chunkX, chunkZ);
    auto it = positionMap_.find(chunkKey);
    if (it == positionMap_.end()) {
        auto& currentChunk = chunks_.emplace_back(MUnique<Chunk>(this));
        generateLandChunk(*currentChunk, chunkX, chunkZ);
        positionMap_[chunkKey] = static_cast<S16>(chunks_.size()) - 1; // index of the new chunk
        return *currentChunk;
    }
    return *chunks_[it->second];
}

#ifdef KIRBY_EASTER_EGG
bool World::shouldPlaceKirby(int localX, int baseY, int localZ, Chunk& chunk) const {
    // Check if the kirby position is within the horizontal bounds of the chunk
    if (localX < 2 || localX >= CHUNK_SIZE - 2 || localZ < 2 || localZ >= CHUNK_SIZE - 2) {
        return false;  // The tree would extend beyond chunk boundaries
    }

    // Check if there is enough height in the chunk to place the kirby
    if (baseY + 2 >= CHUNK_HEIGHT) {
        return false;  // Not enough vertical space for kirby
    }

    // Check No kirby above other block that is not Dirt or Grass
    auto currentCubitoType = chunk.getCubito(CubePosition(localX, baseY - 1, localZ)).type;
    if (currentCubitoType != BLOCK_GRASS && currentCubitoType != BLOCK_DIRT) return false;

    // Check around a 3x3 area to check if kirby can fit in it
    for (s8 x = -1; x <= 1; ++x) {
        for (s8 z = -1; z <= 1; ++z) {
            if(isValidPosition(localX + x, baseY, localZ + z)) {
                auto currentCubitoType2 = chunk.getCubito(CubePosition(localX + x, baseY, localZ + z)).type;
                if (currentCubitoType2 != BLOCK_AIR) return false;
            }else {
                return false;
            }
        }
    }

    // If all checks pass, there is enough space to place the kirby
    return true;
}
#endif

bool World::shouldPlaceTree(int localX, int baseY, int localZ, Chunk& chunk) const {
    // Check if the tree position is within the horizontal bounds of the chunk
    if (localX < 2 || localX >= CHUNK_SIZE - 2 || localZ < 2 || localZ >= CHUNK_SIZE - 2) {
        return false;  // The tree would extend beyond chunk boundaries
    }

    // Check if there is enough height in the chunk to place the entire tree
    if (baseY + 7 >= CHUNK_HEIGHT) {
        return false;  // Not enough vertical space for the tree
    }

    // Check No trees above other block that is not Dirt or Grass
    auto currentCubitoType = chunk.getCubito(CubePosition(localX, baseY - 1, localZ)).type;
    if (currentCubitoType != BLOCK_GRASS && currentCubitoType != BLOCK_DIRT) return false;

    // Verify there are no occupied blocks where the trunk will be placed
    for (int i = 0; i < TRUNK_HEIGHT; ++i) {
        if (chunk.getCubito(CubePosition(localX, baseY + i, localZ)).type != BLOCK_AIR) {
            return false;  // There is something in the space where the trunk should go
        }
    }

    // Check that there are no occupied blocks in the positions where leaves will be placed
    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 2; ++j) {
            // Leaf layers at heights `baseY + 4` and `baseY + 5`
            if (chunk.getCubito(CubePosition(localX + i, baseY + 4, localZ + j)).type != BLOCK_AIR ||
                chunk.getCubito(CubePosition(localX + i, baseY + 5, localZ + j)).type != BLOCK_AIR) {
                return false;  // There is something in the space where leaves should go
                }
        }
    }

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            // Leaf layers at heights `baseY + 3` and `baseY + 6`
            if (chunk.getCubito(CubePosition(localX + i, baseY + 3, localZ + j)).type != BLOCK_AIR ||
                chunk.getCubito(CubePosition(localX + i, baseY + 6, localZ + j)).type != BLOCK_AIR) {
                return false;  // There is something in the space where leaves should go
            }
        }
    }

    // If all checks pass, there is enough space to place the tree
    return true;
}

void World::placeTree(Chunk& chunk, int x, int y, int z) {
    //Trunk:
    for(int i = 0; i < TRUNK_HEIGHT; i++) {
        chunk.setCubito(CubePosition(x, y + i, z), BLOCK_TREE);
    }
    
    //Leaves:
    // Add two smaller leaf layers at heights y + 3 and y + 6
    for (int i = -1; i <= 1; i++) { 
        for (int j = -1; j <= 1; j++) {
            chunk.setCubito(CubePosition(x + i, y + 3, z + j), BLOCK_LEAF);
            chunk.setCubito(CubePosition(x + i, y + 6, z + j), BLOCK_LEAF);
        }
    }
    // Add wider leaf layers at heights y + 4 and y + 5
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            chunk.setCubito(CubePosition(x + i, y + 4, z + j), BLOCK_LEAF);
            chunk.setCubito(CubePosition(x + i, y + 5, z + j), BLOCK_LEAF);
        }
    }
    nTrees_++;
}

int World::getGroundHeight(int worldX, int worldZ) const {
    // Calculate the height at the given world coordinates (worldX, worldZ) using noise
    float height = noiseLite_.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    // Map the noise-based height to a block height within the chunk
    // The formula adjusts the range to fit within the chunk's height (CHUNK_HEIGHT)
    int blockHeight = static_cast<int>((height + 1) * (CHUNK_HEIGHT / 2));
    
    return blockHeight;
}

Chunk* World::getChunk(S16 chunkX, S16 chunkZ) {
    const auto chunkKey = std::make_pair(chunkX, chunkZ);
    auto it = positionMap_.find(chunkKey);
    if (it == positionMap_.end()) {
        return nullptr;
    }
    return chunks_[it->second].get();
}

void World::occludeChunkBlocks() {
    validBlocks_ = 0;
    validFaces_ = 0;
    for(auto& chunk : chunks_) {
        auto valid = chunk->occludeBlocks();
        validBlocks_ += valid.x;
        validFaces_ += valid.y;
    }
}

void World::occludeChunkBlocksFaces() const {
    for(auto& chunk : chunks_) {
        chunk->occludeBlocksFaces();
    }
}

u16 World::renderChunksAround(int playerX, int playerZ, U8* waterTexCoords) {
    int chunkX = playerX / CHUNK_SIZE; 
    int chunkZ = playerZ / CHUNK_SIZE; 

    std::vector<Chunk*> chunksToRender;
    
    for (int x = -CHUNK_LOAD_RADIUS; x <= CHUNK_LOAD_RADIUS; ++x) {
        for (int z = -CHUNK_LOAD_RADIUS; z <= CHUNK_LOAD_RADIUS; ++z) {
            if(auto currentChunk = getChunk(chunkX + x, chunkZ + z)) {
                //currentChunk->render();
                chunksToRender.push_back(currentChunk);
            }
        }
    }

    for(auto& c : chunksToRender) {
        c->render();
    }

    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
        
    GX_SetArray(GX_VA_TEX0, waterTexCoords, 2 * sizeof(u8));
    DCStoreRange(waterTexCoords, 8 * sizeof(u8));
    GX_InvVtxCache();

    for(auto& c : chunksToRender) {
        c->renderTranslucents();
    }
    
    //SYS_Report("N Chunks: %d\n", counter);
    return static_cast<u16>(chunksToRender.size());
}

u16 World::render(U8* waterTexCoords) const {
    for(auto& chunkito : chunks_) {
        chunkito->render();
    }

    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
        
    GX_SetArray(GX_VA_TEX0, waterTexCoords, 2 * sizeof(u8));
    DCStoreRange(waterTexCoords, 8 * sizeof(u8));
    GX_InvVtxCache();

    for(auto& chunkito : chunks_) {
        if(!chunkito->displayListTransparent) continue;
        chunkito->renderTranslucents();
    }

    return static_cast<u16>(chunks_.size());
}
