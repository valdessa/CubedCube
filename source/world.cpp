#include <common.h>

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

#include <chunk.h>

using namespace poyo;

World::World() {
    //noiseLite_.SetSeed(seed);
    noiseLite_.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noiseLite_.SetFrequency(0.05f);
}

World::~World() {
    //todo: destroy display lists
}

void World::generateLand(S16 radius) {
    for (int chunkX = -radius; chunkX <= radius; ++chunkX) {
        for (int chunkZ = -radius; chunkZ <= radius; ++chunkZ) {

            validBlocks_ += getOrCreateChunkForLand(
                static_cast<S16>(chunkX),
                static_cast<S16>(chunkZ)).validBlocks;
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

    // // Second pass: Place water in dips, one block below grass level to create a pool effect
    // for (int x = 0; x < CHUNK_SIZE; ++x) {
    //     for (int z = 0; z < CHUNK_SIZE; ++z) {
    //         int worldX = x + (chunkX * CHUNK_SIZE);
    //         int worldZ = z + (chunkZ * CHUNK_SIZE);
    //         int groundHeight = getGroundHeight(worldX, worldZ);
    //
    //         // Only place water if there’s a dip just below the ground level
    //         if (groundHeight - 1 > 0) {  // Ensure we're within bounds
    //             CubePosition waterPos(x, groundHeight - 1, z);
    //             CubePosition aboveWaterPos(x, groundHeight, z);
    //
    //             // Place water if the position directly below the ground is empty (a dip)
    //             if (chunk.getCubito(aboveWaterPos).type == BLOCK_AIR) {
    //                 chunk.setCubito(waterPos, BLOCK_WATER); // Water one block below grass, only in dips
    //             }
    //         }
    //     }
    // }

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
    for(auto& chunk : chunks_) {
        validBlocks_ += chunk->occludeBlocks();
    }
}

void World::occludeChunkBlocksFaces() const {
    for(auto& chunk : chunks_) {
        chunk->occludeBlocksFaces();
    }
}

void World::renderChunksAround(int playerX, int playerZ) {
    int chunkX = playerX / CHUNK_SIZE; 
    int chunkZ = playerZ / CHUNK_SIZE; 

    int counter = 0;
    for (int x = -CHUNK_LOAD_RADIUS; x <= CHUNK_LOAD_RADIUS; ++x) {
        for (int z = -CHUNK_LOAD_RADIUS; z <= CHUNK_LOAD_RADIUS; ++z) {
            if(auto currentChunk = getChunk(chunkX + x, chunkZ + z)) {
                currentChunk->render();
                counter++;
            }
        }
    }
    //SYS_Report("N Chunks: %d\n", counter);
}
