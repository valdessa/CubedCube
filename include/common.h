#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_ 1

#include <typedefs.h>

#define CHUNK_SIZE   16   // Ancho de cada chunk (X, Z)
#define CHUNK_HEIGHT 16  // Altura de cada chunk (Y)

#define MIN_HEIGHT  2

#define WATER_LEVEL 8        // Nivel del agua
#define DIRT_LEVEL 2         // Capas de tierra bajo el césped
#define STONE_LEVEL 4        // Capas de roca bajo la tierra
#define GRASS_LEVEL (WATER_LEVEL + DIRT_LEVEL) // Capa de césped

#define MAX_TREES 2
#define TRUNK_HEIGHT 4

const int CHUNK_LOAD_RADIUS = 2;

constexpr poyo::U8 CHUNK_RADIUS = 6;

/***** OCCLUSION CULLING OPTIMIZATIONS *****/                                              
//0 -> NO OCCLUSION CULLING :(                          
//1 -> OCCLUSION CULLING ONLY BLOCKS   (GAME LOOP)      
//2 -> OCCLUSION CULLING BLOCK + FACES (GAME LOOP)     
//3 -> OCCLUSION CULLING ONLY BLOCKS   (PRECALCULATED)  
//4 -> OCCLUSION CULLING BLOCKS FACES  (PRECALCULATED) 

#define OPTIMIZATION_VECTOR
#define OPTIMIZATION_MAPS
#define OPTIMIZATION_OCCLUSION 4
#define OPTIMIZATION_BATCHING       //TODO: IMPROVE THIS OPTION!!
#define OPTIMIZATION_DISPLAY_LIST
#define OPTIMIZATION_STRUCTS

/*  
=============================================================== BENCHMARKING ==================================================================|
                                                                                                                                               |
    OCCLUSION TYPE                                 | DEFAULT                    | DISPLAY LIST              | DISPLAY LISTS + OPTIMIZE STRUCTS |  
-----------------------------------------------------------------------------------------------------------------------------------------------|
-> NO OCCLUSION CULLING :(                         | R. 1: 9   Chunks (16 fps)  | R. 1: 9  Chunks (60 fps)  | R. 1: 9   Chunks (60 fps)        |                
-> OCCLUSION CULLING ONLY BLOCKS   (GAME LOOP)     | R. 1: 9   Chunks (30 fps)  | -                         | -                                |       
-> OCCLUSION CULLING BLOCK + FACES (GAME LOOP)     | R. 1: 9   Chunks (30 fps)  | -                         | -                                |  
-> OCCLUSION CULLING ONLY BLOCKS   (PRECALCULATED) | R. 4: 81  Chunks (12 fps)  | R. 3: 49  Chunks (60 fps) | R. 4: 81  Chunks (60 fps)        |                     
-> OCCLUSION CULLING BLOCK + FACES (PRECALCULATED) | -                          | R. 4: 81  Chunks (60 fps) | R. 6: 169 Chunks (60 fps)        |                      
                                                                                                                                               |
===============================================================================================================================================|
*/


#ifdef OPTIMIZATION_DISPLAY_LIST
    #if OPTIMIZATION_OCCLUSION == 1 || OPTIMIZATION_OCCLUSION == 2
        #error "OPTIMIZATION DISPLAY LIST CAN NOT BE USED WITH THIS OCCLUSION OPTION"
    #endif
#endif

namespace poyo {
    struct ChunkPosition{
        S16 x, z;
    };
    
    using CubePosition = SVec3;
    
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
        BLOCK_LEAF2,
        BLOCK_CACTUS,
        BLOCK_SAND_DIRT,

        //BLOCK_WATER, //TODO: FIX THIS
        
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
        TILE_LEAF_TRANS  = 13,
        TILE_CACTUS_BOT  = 18,
        TILE_CACTUS_SIDE = 19,
        TILE_CACTUS_TOP  = 20,
        NUM_TILES,
    };

#ifdef OPTIMIZATION_STRUCTS
    struct CubeFace {   //Size = 2 bytes
        U8 x : 1;           // 1 bit for x (0-1)
        U8 y : 1;           // 1 bit for y (0-1)
        U8 z : 1;           // 1 bit for z (0-1)
        U8 direction : 4;   // 4 bits for direction (0-15)
        U8 tile : 8;        // 1 byte for tile (0-255)
    };
    struct Cubito {     //Size = 16 bytes
        CubeFace face[6];   // 6 CubeFace (12 bytes total, packed)
        S8 x, y, z;         // 3 bytes (total 24 bits)
        U8 type : 7;        // 7 bits for the block type (0-127)
        U8 visible : 1;     // 1 bit for visibility (0-1)

        Cubito() : face{}, x(0), y(0), z(0), type(BLOCK_AIR), visible(false) {
            
        }
    };
#else
    struct CubeFace {   //Size = 5 bytes
        U8 x, y, z;           
        U8 direction;   
        U8 tile;        
    };
    struct Cubito {     //Size = 38 bytes
        S16 x, y, z;
        CubeFace face[6];
        U8 type = BLOCK_AIR;
        bool visible = false;
    };
#endif

    inline constexpr U8 blockTiles[][6] = {
        [BLOCK_STONE] =    {TILE_STONE,      TILE_STONE,      TILE_STONE,    TILE_STONE,     TILE_STONE,      TILE_STONE},
        [BLOCK_SAND] =     {TILE_SAND,       TILE_SAND,       TILE_SAND,     TILE_SAND,      TILE_SAND,       TILE_SAND},
        [BLOCK_DIRT] =     {TILE_DIRT,       TILE_DIRT,       TILE_DIRT,     TILE_DIRT,      TILE_DIRT,       TILE_DIRT},
        [BLOCK_GRASS] =    {TILE_GRASS_SIDE, TILE_GRASS_SIDE, TILE_GRASS,    TILE_DIRT,      TILE_GRASS_SIDE, TILE_GRASS_SIDE},
        [BLOCK_WOOD] =     {TILE_WOOD,       TILE_WOOD,       TILE_WOOD,     TILE_WOOD,      TILE_WOOD,       TILE_WOOD},
        [BLOCK_TREE] =     {TILE_TREE_SIDE,  TILE_TREE_SIDE,  TILE_TREE_TOP,   TILE_TREE_TOP,   TILE_TREE_SIDE,   TILE_TREE_SIDE},
        [BLOCK_LEAF] =     {TILE_LEAF,       TILE_LEAF,       TILE_LEAF,       TILE_LEAF,       TILE_LEAF,        TILE_LEAF},
        [BLOCK_LEAF2] =    {TILE_LEAF_TRANS, TILE_LEAF_TRANS, TILE_LEAF_TRANS, TILE_LEAF_TRANS, TILE_LEAF_TRANS,  TILE_LEAF_TRANS},
        [BLOCK_CACTUS] =   {TILE_CACTUS_SIDE, TILE_CACTUS_SIDE, TILE_CACTUS_TOP, TILE_CACTUS_BOT, TILE_CACTUS_SIDE, TILE_CACTUS_SIDE},
        [BLOCK_SAND_DIRT] ={TILE_SAND_DIRT, TILE_SAND_DIRT, TILE_SAND, TILE_DIRT, TILE_SAND_DIRT, TILE_SAND_DIRT},

        [BLOCK_AIR] = {TILE_STONE,      TILE_STONE,      TILE_STONE,    TILE_STONE,     TILE_STONE,      TILE_STONE},
    };

    inline constexpr U8 cubeFaces[6][4][3] = {
        [DIR_X_FRONT] = {{0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1}},  //drawn clockwise looking x-
        [DIR_X_BACK] =  {{0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}},  //drawn clockwise looking x+
        [DIR_Y_FRONT] = {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},  //drawn clockwise looking y-
        [DIR_Y_BACK] =  {{0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0}},  //drawn clockwise looking y+
        [DIR_Z_FRONT] = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}},  //drawn clockwise looking z-
        [DIR_Z_BACK] =  {{1, 1, 0}, {0, 1, 0}, {0, 0, 0}, {1, 0, 0}},  //drawn clockwise looking z+
    };
    
    inline constexpr S8 cubeNormals[6][3] = {
        [DIR_X_FRONT] = { 1,  0,  0},  // Front (X+)
        [DIR_X_BACK]  = {-1,  0,  0},  // Back (X-)
        [DIR_Y_FRONT] = { 0,  1,  0},  // Top (Y+)
        [DIR_Y_BACK]  = { 0, -1,  0},  // Bottom (Y-)
        [DIR_Z_FRONT] = { 0,  0,  1},  // Left (Z+)
        [DIR_Z_BACK]  = { 0,  0, -1},  // Right (Z-)
    };
    
#ifdef OPTIMIZATION_MAPS
    inline U16 tileUVMap[NUM_TILES][2]{};
#else
    inline Map<U8, USVec2> tileUVMap;
#endif
}

// Bits -> U(0<->Max Value) S(Min Value<->Max Value)
// 0   -> U(0<->0)     S(0<->0)
// 1   -> U(0<->1)     S(-1<->0)
// 2   -> U(0<->3)     S(-2<->1)
// 3   -> U(0<->7)     S(-4<->3)
// 4   -> U(0<->15)    S(-8<->7)
// 5   -> U(0<->31)    S(-16<->15)
// 6   -> U(0<->63)    S(-32<->31)
// 7   -> U(0<->127)   S(-64<->63)
// 8   -> U(0<->255)   S(-128<->127)
// 9   -> U(0<->511)   S(-256<->255)
// 10  -> U(0<->1023)  S(-512<->511)
// 11  -> U(0<->2047)  S(-1024<->1023)
// 12  -> U(0<->4095)  S(-2048<->2047)
// 13  -> U(0<->8191)  S(-4096<->4095)
// 14  -> U(0<->16383) S(-8192<->8191)
// 15  -> U(0<->32767) S(-16384<->16383)
// 16  -> U(0<->65535) S(-32768<->32767)

#endif