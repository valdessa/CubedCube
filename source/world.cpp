#include <common.h>

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
    
}

World::~World() {
}

void World::generateChunk(Chunk& chunk, S16 chunkX, S16 chunkZ) {
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

Chunk& World::getOrCreateChunk(S16 chunkX, S16 chunkZ) {
    // Cargar o crear el chunk si no existe en esa posición
    const auto chunkKey = std::make_pair(chunkX, chunkZ);
    if (chunks_.find(chunkKey) == chunks_.end()) {
        // Si no existe, lo generamos
        auto& NewChunk = chunks_.emplace(chunkKey, Chunk()).first->second;
        generateChunk(NewChunk, chunkX, chunkZ);
        return NewChunk;
    }
    return chunks_[chunkKey];
}

float World::getHeightAt(S32 x, S32 z) {
    // Una simple función de terreno ondulado usando senos
    float height = 5.0f * sinf(x * 0.1f) + 5.0f * cosf(z * 0.1f);
    return std::max(static_cast<float>(MIN_HEIGHT), height + 10.0f);
}
