#include <common.h>
#include <ogc/system.h>

namespace std {
    template<>
    struct hash<std::pair<short, short>> {
        std::size_t operator()(const std::pair<short, short>& p) const noexcept {
            // Combina los hashes de ambos elementos del pair
            std::size_t h1 = std::hash<short>{}(p.first);
            std::size_t h2 = std::hash<short>{}(p.second);
            return h1 ^ (h2 << 1); // Combina los hashes de forma simple
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
}

void World::generateLand(S16 radius) {
    for (int chunkX = -radius; chunkX <= radius; ++chunkX) {
        for (int chunkZ = -radius; chunkZ <= radius; ++chunkZ) {

            validBlocks_ += getOrCreateChunkForLand(chunkX, chunkZ).validBlocks;
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

void World::generateLandChunk(Chunk& chunk, S16 chunkX, S16 chunkZ) {
    chunk.offsetPosition_.x = chunkX;
    chunk.offsetPosition_.z = chunkZ;
    chunk.worldPosition_.x = chunkX * CHUNK_SIZE;
    chunk.worldPosition_.z = chunkZ * CHUNK_SIZE;
    
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Calculamos la posición global en el mundo
            int worldX = x + (chunkX * CHUNK_SIZE);
            int worldZ = z + (chunkZ * CHUNK_SIZE);

            // Generamos una altura basada en ruido para la posición (x, z)
            float height = noiseLite_.GetNoise((float)worldX, (float)worldZ);
            int blockHeight = (int)((height + 1) * (CHUNK_HEIGHT / 2));

            // Asignamos bloques según la altura generada
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                CubePosition pos(x, y, z);
                if (y > blockHeight) {
                    chunk.setCubito(pos, BLOCK_AIR); // Espacio vacío
                } else if (y == blockHeight) {
                    chunk.setCubito(pos, BLOCK_GRASS);  // Césped en la cima
                } else if (y > blockHeight - DIRT_LEVEL) {
                    chunk.setCubito(pos, BLOCK_DIRT);   // Tierra debajo del césped
                } else if (y > blockHeight - STONE_LEVEL) {
                    chunk.setCubito(pos, BLOCK_STONE);  // Roca debajo de la tierra
                } else {
                    chunk.setCubito(pos, BLOCK_STONE); // Todo lo demás es roca
                }
            }
        }
    }

    for(int i = 0; i < MAX_TREES; i++) {
        // Generamos una posición aleatoria dentro del chunk
        int x = rand() % CHUNK_SIZE;
        int z = rand() % CHUNK_SIZE;

        // Calculamos la posición global en el mundo
        int worldX = x + (chunkX * CHUNK_SIZE);
        int worldZ = z + (chunkZ * CHUNK_SIZE);
        int randY = getGroundHeight(worldX, worldZ, chunk); // Obtén la altura del terreno en (randX, randZ)

        // Verificar si podemos colocar un árbol en esa posición
        if (shouldPlaceTree(x, randY + 1, z, chunk)) {
            // Colocar el árbol si es posible
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
        positionMap_[chunkKey] = chunks_.size() - 1; // index of the new chunk
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

int World::getGroundHeight(int worldX, int worldZ, const Chunk& chunk) const {
    // Calculate the height at the given world coordinates (worldX, worldZ) using noise
    float height = noiseLite_.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
    
    // Map the noise-based height to a block height within the chunk
    // The formula adjusts the range to fit within the chunk's height (CHUNK_HEIGHT)
    int blockHeight = (int)((height + 1) * (CHUNK_HEIGHT / 2));
    
    return blockHeight;
}

void World::generateChunks(S16 middleX, S16 middleZ, S16 numChunksX, S16 numChunksZ) {

    S16 halfChunk = CHUNK_SIZE / 2;
    S16 offsetX = halfChunk * numChunksX;
    S16 offsetZ = halfChunk * numChunksZ;
    
    // Recorremos la región de terreno especificada
    for (S16 chunkX = middleX; chunkX < middleX + numChunksX; ++chunkX) {
        for (S16 chunkZ = middleZ; chunkZ < middleZ + numChunksZ; ++chunkZ) {

            auto worldX = chunkX * CHUNK_SIZE - offsetX;
            auto worldZ = chunkZ * CHUNK_SIZE - offsetZ;
            
            // Genera o recupera el chunk de la posición actual
            validBlocks_ += getOrCreateChunk(worldX, worldZ).validBlocks;
        }
    }
}

void World::generateChunk(Chunk& chunk, S16 chunkX, S16 chunkZ) {
    chunk.worldPosition_.x = chunkX;
    chunk.worldPosition_.z = chunkZ;
    for (S16 x = 0; x < CHUNK_SIZE; ++x) {
        for (S16 z = 0; z < CHUNK_SIZE; ++z) {
            S32 worldX = chunkX * CHUNK_SIZE + x;
            S32 worldZ = chunkZ * CHUNK_SIZE + z;

            //float height = getHeightAt(worldX, worldZ);
            int height = static_cast<int>(round(getHeightAt(worldX, worldZ)));

            for (int y = 0; y < CHUNK_SIZE; ++y) {
                CubePosition pos = CubePosition{x, y, z};
                if (y < height) {
                    // Determinar tipo de bloque según la altura
                    if (y < height - STONE_LEVEL) {
                        chunk.setCubito(pos, BLOCK_STONE); // Piedra en capas profundas
                    } else if (y < height - GRASS_LEVEL) {
                        chunk.setCubito(pos, BLOCK_DIRT);  // Tierra en las capas superiores
                    } else if (y == height - GRASS_LEVEL) {
                        chunk.setCubito(pos, BLOCK_GRASS); // Bloque en la superficie -> césped
                    } else {
                        helperCounter++;
                        chunk.setCubito(pos, BLOCK_AIR);
                    }
                } else {
                    chunk.setCubito(pos, BLOCK_AIR); // Aire por encima
                }
            }
        }
    }
}

void World::generateSolidChunk(Chunk& chunk, S16 chunkX, S16 chunkZ) {
    chunk.worldPosition_.x = chunkX;
    chunk.worldPosition_.z = chunkZ;
    for (S16 x = 0; x < CHUNK_SIZE; ++x) {
        for(S16 y = 0; y < CHUNK_SIZE; y++) {
            for (S16 z = 0; z < CHUNK_SIZE; ++z) {
                S32 worldX = chunkX * CHUNK_SIZE + x;
                S32 worldZ = chunkZ * CHUNK_SIZE + z;
                CubePosition pos = CubePosition{x, y, z};
                // Determinar tipo de bloque según la altura
                if (y < STONE_LEVEL) {
                    chunk.setCubito(pos, BLOCK_STONE); // Piedra en capas profundas
                } else if (y < GRASS_LEVEL) {
                    chunk.setCubito(pos, BLOCK_DIRT);  // Tierra en las capas superiores
                } else if (y == GRASS_LEVEL) {
                    chunk.setCubito(pos, BLOCK_GRASS); // Bloque en la superficie -> césped
                } else {
                    chunk.setCubito(pos, BLOCK_SAND);
                }
            } 
        }
    }
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
    int chunkX = playerX / CHUNK_SIZE; // Divide para obtener el índice del chunk en X
    int chunkZ = playerZ / CHUNK_SIZE; // Divide para obtener el índice del chunk en Z

    int counter = 0;
    for (int x = -CHUNK_LOAD_RADIUS; x <= CHUNK_LOAD_RADIUS; ++x) {
        for (int z = -CHUNK_LOAD_RADIUS; z <= CHUNK_LOAD_RADIUS; ++z) {
            auto currentChunk = getChunk(chunkX + x, chunkZ + z);
            if(currentChunk) {
                currentChunk->render();
                counter++;
            }
        }
    }
    //SYS_Report("N Chunks: %d\n", counter);
}

// Chunk& World::getOrCreateChunk(S16 chunkX, S16 chunkZ) {
//     // Cargar o crear el chunk si no existe en esa posición
//     const auto chunkKey = std::make_pair(chunkX, chunkZ);
//     if (positionMap_.find(chunkKey) == positionMap_.end()) {
//         // Si no existe, lo generamos
//         auto& NewChunk = positionMap_.emplace(chunkKey, Chunk()).first->second;
//         generateChunk(NewChunk, chunkX, chunkZ);
//         return NewChunk;
//     }
//     return positionMap_[chunkKey];
// }

float World::getHeightAt(S32 x, S32 z) {
    // Una simple función de terreno ondulado usando senos
    //float height = 5.0f * sinf(x * 0.1f) + 5.0f * cosf(z * 0.1f);
    float height = 5.0f * noise_.perlin(x * 0.2f, z * 0.2f);
    return std::max(static_cast<float>(MIN_HEIGHT), height + 10.0f);
}
