#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_ 1

#include <typedefs.h>

#define CHUNK_SIZE 16
#define WATER_LEVEL 5 
#define GRASS_LEVEL 10
#define STONE_LEVEL 12
#define MIN_HEIGHT  2

#define OPTIMIZE_VECTOR
#define OPTIMIZE_MAPS

namespace poyo {
    using CubePosition = SVec3;

    struct CubeFace {
        U8 x, y, z;
        U8 direction;
        U8 tile;
    };
    
    struct Cubito {
        CubeFace face[6];
        S16 x, y, z;
        U8 type;
    };

    enum {
        DIR_X_FRONT,
        DIR_X_BACK,
        DIR_Y_FRONT,
        DIR_Y_BACK,
        DIR_Z_FRONT,
        DIR_Z_BACK,
    };

    enum BLOCK_TYPE {
        BLOCK_STONE,
        BLOCK_SAND,
        BLOCK_DIRT,
        BLOCK_GRASS,
        BLOCK_WOOD,
        BLOCK_TREE,
        BLOCK_LEAF,
        BLOCK_CACTUS,
        BLOCK_SAND_DIRT,
        BLOCK_AIR //USED FOR COUNT/NOT BLOCK 
    };

    enum {
        TILE_DIRT        = 0,
        TILE_GRASS_SIDE  = 1,
        TILE_GRASS       = 2,

        TILE_STONE       = 3,

        TILE_SAND        = 6,
        TILE_SAND_DIRT   = 7,
    
        TILE_TREE_SIDE   = 9,
        TILE_TREE_TOP    = 10,
        TILE_WOOD        = 11,
        TILE_LEAF        = 12,
        TILE_CACTUS_BOT  = 18,
        TILE_CACTUS_SIDE = 19,
        TILE_CACTUS_TOP  = 20,
        NUM_TILES,
    };

    static const U8 blockTiles[][6] = {
        [BLOCK_STONE] =    {TILE_STONE,      TILE_STONE,      TILE_STONE,    TILE_STONE,     TILE_STONE,      TILE_STONE},
        [BLOCK_SAND] =     {TILE_SAND,       TILE_SAND,       TILE_SAND,     TILE_SAND,      TILE_SAND,       TILE_SAND},
        [BLOCK_DIRT] =     {TILE_DIRT,       TILE_DIRT,       TILE_DIRT,     TILE_DIRT,      TILE_DIRT,       TILE_DIRT},
        [BLOCK_GRASS] =    {TILE_GRASS_SIDE, TILE_GRASS_SIDE, TILE_GRASS,    TILE_DIRT,      TILE_GRASS_SIDE, TILE_GRASS_SIDE},
        [BLOCK_WOOD] =     {TILE_WOOD,       TILE_WOOD,       TILE_WOOD,     TILE_WOOD,      TILE_WOOD,       TILE_WOOD},
        [BLOCK_TREE] =     {TILE_TREE_SIDE,  TILE_TREE_SIDE,  TILE_TREE_TOP, TILE_TREE_TOP,  TILE_TREE_SIDE,  TILE_TREE_SIDE},
        [BLOCK_LEAF] =     {TILE_LEAF,       TILE_LEAF,       TILE_LEAF,     TILE_LEAF,      TILE_LEAF,       TILE_LEAF},
        [BLOCK_CACTUS] =   {TILE_CACTUS_SIDE, TILE_CACTUS_SIDE, TILE_CACTUS_TOP, TILE_CACTUS_BOT, TILE_CACTUS_SIDE, TILE_CACTUS_SIDE},
        [BLOCK_SAND_DIRT] ={TILE_SAND_DIRT, TILE_SAND_DIRT, TILE_SAND, TILE_DIRT, TILE_SAND_DIRT, TILE_SAND_DIRT},
    };

    static const U8 cubeFaces[6][4][3] = {
        [DIR_X_FRONT] = {{0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1}},  //drawn clockwise looking x-
        [DIR_X_BACK] =  {{0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}},  //drawn clockwise looking x+
        [DIR_Y_FRONT] = {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},  //drawn clockwise looking y-
        [DIR_Y_BACK] =  {{0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0}},  //drawn clockwise looking y+
        [DIR_Z_FRONT] = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}},  //drawn clockwise looking z-
        [DIR_Z_BACK] =  {{1, 1, 0}, {0, 1, 0}, {0, 0, 0}, {1, 0, 0}},  //drawn clockwise looking z+
    };
}

#endif