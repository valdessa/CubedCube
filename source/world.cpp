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

#ifdef OPTIMIZATION_OCCLUSION_PRECALCULATED
    occludeChunks();
#endif

#ifdef OPTIMIZATION_DISPLAY_LIST
    for(auto& chunks : getChunks()) {
        chunks->createDisplayList();
    }
#endif
}

void World::generateLandChunk(Chunk& chunk, S16 chunkX, S16 chunkZ) const {
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
}

Chunk& World::getOrCreateChunkForLand(S16 chunkX, S16 chunkZ) {
    const auto chunkKey = std::make_pair(chunkX, chunkZ);
    auto it = positionMap_.find(chunkKey);
    if (it == positionMap_.end()) {
        auto& currentChunk = chunks_.emplace_back(MUnique<Chunk>(this));
        generateLandChunk(*currentChunk, chunkX, chunkZ);
        positionMap_[chunkKey] = chunks_.size() - 1; // índice del nuevo chunk
        return *currentChunk;
    }
    return *chunks_[it->second];
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

void World::occludeChunks() {
    validBlocks_ = 0;
    for(auto& chunk : chunks_) {
        for(auto& cubito : chunk->cubitos_) {
            if(!chunk->isSolid(cubito)) {
                cubito.visible = false;
                continue;
            }
            cubito.visible = !chunk->isCompletelyOccluded(cubito.x, cubito.y, cubito.z, chunk->offsetPosition_);
            if(cubito.visible) validBlocks_++;
        }
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
