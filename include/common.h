#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_ 1

#include <typedefs.h>

#define CHUNK_SIZE   16  // Width  of each chunk (X, Z)
#define CHUNK_HEIGHT 16  // Height of each chunk (Y)

#define WATER_LEVEL 3       
#define DIRT_LEVEL  2         
#define STONE_LEVEL 4        
#define GRASS_LEVEL 5

#define MAX_TREES 2
#define MAX_FLOWERS 4
#define MAX_HERBS 2
#define TRUNK_HEIGHT 4

inline int CHUNK_LOAD_RADIUS = 2;

/***** OCCLUSION CULLING OPTIMIZATIONS *****/                                              
//0 -> NO OCCLUSION CULLING :(                         20.135/120.594
//1 -> OCCLUSION CULLING ONLY BLOCKS   (GAME LOOP)     3.985/23.694
//2 -> OCCLUSION CULLING BLOCK + FACES (GAME LOOP)     23.694/23.694 ->9/5582   only for batching!!
//3 -> OCCLUSION CULLING ONLY BLOCKS   (PRECALCULATED)  3.985/23.694
//4 -> OCCLUSION CULLING BLOCKS FACES  (PRECALCULATED)  3.985/23.694 ->9/5582   only for batching!!

//measures with ticks and ms
//full frame + only draw part

#define OPTIMIZATION_VECTOR
#define OPTIMIZATION_MAPS

#if USE_MAKEFILE_DEFINES == 0
     #define CHUNK_RADIUS 8
     #define OPTIMIZATION_OCCLUSION 4
     #define OPTIMIZATION_BATCHING       
     #define OPTIMIZATION_DISPLAY_LIST
     #define OPTIMIZATION_STRUCTS 2
     #define OPTIMIZATION_MODEL_MATRIX
     #define OPTIMIZATION_VERTEX_MEMORY 
     #define OPTIMIZATION_NO_LIGHTNING_DATA 
     #define CHUNK_RENDER_MODE 2
#endif


#define OPTIMIZATION_FRUSTUM_CULLING
///-----
#define TO_STRING(x) #x
#define TOSTRING(x) TO_STRING(x)
///-----
///
#define CHUNK_RADIUS_STRING "RAD_" TOSTRING(CHUNK_RADIUS)
#define OCCLUSION_STRING "OCC_" TOSTRING(OPTIMIZATION_OCCLUSION)
#define STRUCT_STRING "STRUC_" TOSTRING(OPTIMIZATION_STRUCTS)
#define CHUNK_RENDER_STRING "RM_" TOSTRING(CHUNK_RENDER_MODE)

#define KIRBY_EASTER_EGG
#define KIRBY_IN_DISPLAY_LIST
#define KIRBY_CONTROLLED
#define MAX_KIRBY 64

enum class FileFormat {
    TXT,
    CSV
};

//#define ENABLE_AUTOMATIC_CAMERA 2 //seconds
//#define ENABLE_MEASUREMENTS
#define MEASUREMENTS_FRAMES 256
constexpr FileFormat MEASUREMENTS_FILE_FORMAT = FileFormat::CSV;
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
#ifndef OPTIMIZATION_BATCHING
    #if OPTIMIZATION_OCCLUSION == 2 || OPTIMIZATION_OCCLUSION == 4
    #error "THE SELECTED OPTIMIZATION OCCLUSSION OPTION CANNOT BE USED WITHOUT BATCHING"
    #endif
#endif

#ifdef OPTIMIZATION_DISPLAY_LIST
    #if OPTIMIZATION_OCCLUSION == 1 || OPTIMIZATION_OCCLUSION == 2
        #error "OPTIMIZATION DISPLAY LIST CAN NOT BE USED WITH THIS OCCLUSION OPTION"
    #endif
#endif

#ifndef KIRBY_EASTER_EGG
    #undef KIRBY_IN_DISPLAY_LIST
    #undef KIRBY_CONTROLLED
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

        DIR_DIAG_XY_FRONT,        // Diagonal en el plano XY (esquina superior izquierda a inferior derecha)
        DIR_DIAG_XY_BACK          // Diagonal en el plano XY (esquina superior derecha a inferior izquierda)
    };

    enum BLOCK_TYPE {
        BLOCK_STONE,
        BLOCK_SAND,
        BLOCK_DIRT,
        BLOCK_GRASS,
        BLOCK_WOOD,
        BLOCK_TREE,
        BLOCK_LEAF,
        BLOCK_LEAF_FLOWER,
        BLOCK_WATER,
        BLOCK_CACTUS,

        BLOCK_POPPY,
        BLOCK_ORCHID,
        BLOCK_DANDELION,
        BLOCK_HERB,
        
        BLOCK_AIR //USED FOR COUNT/NOT BLOCK 
    };

    enum {
        TILE_DIRT        = 0,       //Fixed
        TILE_GRASS_SIDE  = 1,       //Fixed
        TILE_GRASS       = 2,       //Fixed

        TILE_STONE       = 7,       //Fixed

           
    
        TILE_TREE_SIDE   = 8,       //Fixed
        TILE_TREE_TOP    = 9,       //Fixed
        TILE_WOOD        = 10,      //Fixed
        
        TILE_LEAF        = 11,      //Fixed
        TILE_LEAF_FL     = 15,      //Fixed
        
        TILE_GLASS       = 14,
        
        TILE_SAND        = 16,      //Fixed
        TILE_CACTUS_BOT  = 17,      //Fixed
        TILE_CACTUS_SIDE = 18,      //Fixed
        TILE_CACTUS_TOP  = 19,      //Fixed

        TILE_ICE         = 20,      //Fixed
        TILE_WATER       = 21,      //Fixed

        TILE_HERB        = 22,      //Fixed 34
        TILE_ORCHID      = 23,      //Fixed 32
        TILE_DANDELION   = 25,      //Fixed 33
        TILE_POPPY       = 28,      //Fixed
        
        NUM_TILES,                  //ALSO A INVALID TILE
    };

    enum BlockProperties {  //I Can have until 8 Properties
        SOLID   = 1 << 0,  // 00000001
        TRIGGER = 1 << 1,  // 00000010
        WATER   = 1 << 2,  // 00000100
        FOLIAGE = 1 << 3   // 00001000
    };

#if OPTIMIZATION_STRUCTS == 0
    struct CubeFace {   //Size = 5 bytes
        U8 x, y, z;           
        U8 direction;   
        U8 tile;        
    };
    struct Cubito {     //Size = 38 bytes
        CubeFace face[6];
        S16 x, y, z;
        U8 type;
        bool visible;

        Cubito() : face{}, x(0), y(0), z(0), type(BLOCK_AIR), visible(false) {
            
        }
    };
#elif OPTIMIZATION_STRUCTS == 1
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
#elif OPTIMIZATION_STRUCTS == 2
    struct CubeFace {   //Size = 1 bytes
        U8 direction : 3;   // 3 bits for direction (0-7)
        U8 tile : 5;        // 5 bits for tile (0-31)
    };
    
    struct Cubito {     //Size = 10 bytes
        CubeFace face[6];   // 6 CubeFace (6 bytes total, packed)
        S8 x, y, z;         // 3 bytes (total 24 bits)
        U8 type : 7;        // 7 bits for the block type (0-127)
        U8 visible : 1;     // 1 bit for visibility (0-1)

        Cubito() : face{}, x(0), y(0), z(0), type(BLOCK_AIR), visible(false) {
            
        }
    };
#endif
    
    inline constexpr U8 blockTiles[][6] = {
        [BLOCK_STONE]       = {TILE_STONE,       TILE_STONE,       TILE_STONE,      TILE_STONE,      TILE_STONE,       TILE_STONE},
        [BLOCK_SAND]        = {TILE_SAND,        TILE_SAND,        TILE_SAND,       TILE_SAND,       TILE_SAND,        TILE_SAND},
        [BLOCK_DIRT]        = {TILE_DIRT,        TILE_DIRT,        TILE_DIRT,       TILE_DIRT,       TILE_DIRT,        TILE_DIRT},
        [BLOCK_GRASS]       = {TILE_GRASS_SIDE,  TILE_GRASS_SIDE,  TILE_GRASS,      TILE_DIRT,       TILE_GRASS_SIDE,  TILE_GRASS_SIDE},
        [BLOCK_WOOD]        = {TILE_WOOD,        TILE_WOOD,        TILE_WOOD,       TILE_WOOD,       TILE_WOOD,        TILE_WOOD},
        [BLOCK_TREE]        = {TILE_TREE_SIDE,   TILE_TREE_SIDE,   TILE_TREE_TOP,   TILE_TREE_TOP,   TILE_TREE_SIDE,   TILE_TREE_SIDE},
        [BLOCK_LEAF]        = {TILE_LEAF,        TILE_LEAF,        TILE_LEAF,       TILE_LEAF,       TILE_LEAF,        TILE_LEAF},
        [BLOCK_LEAF_FLOWER] = {TILE_LEAF_FL,     TILE_LEAF_FL,     TILE_LEAF_FL,    TILE_LEAF_FL,    TILE_LEAF_FL,     TILE_LEAF_FL},
        [BLOCK_WATER]       = {TILE_WATER,       TILE_WATER,       TILE_WATER,      TILE_WATER,      TILE_WATER,       TILE_WATER},
        [BLOCK_CACTUS]      = {TILE_CACTUS_SIDE, TILE_CACTUS_SIDE, TILE_CACTUS_TOP, TILE_CACTUS_BOT, TILE_CACTUS_SIDE, TILE_CACTUS_SIDE},

        [BLOCK_POPPY]       = {TILE_POPPY,       TILE_POPPY,       TILE_POPPY,      TILE_POPPY,      TILE_POPPY,       TILE_POPPY},
        [BLOCK_ORCHID]      = {TILE_ORCHID,      TILE_ORCHID,      TILE_ORCHID,     TILE_ORCHID,     TILE_ORCHID,      TILE_ORCHID},
        [BLOCK_DANDELION]   = {TILE_DANDELION,   TILE_DANDELION,   TILE_DANDELION,  TILE_DANDELION,  TILE_DANDELION,   TILE_DANDELION},
        [BLOCK_HERB]        = {TILE_HERB,        TILE_HERB,        TILE_HERB,       TILE_HERB,       TILE_HERB,        TILE_HERB},
        
        [BLOCK_AIR]         = {NUM_TILES,      NUM_TILES,      NUM_TILES,    NUM_TILES,     NUM_TILES,      NUM_TILES},
    };

    inline constexpr U8 BLOCK_PROPERTIES[] = {
        [BLOCK_STONE]       = SOLID,              
        [BLOCK_SAND]        = SOLID,                   // BLOCK_SAND
        [BLOCK_DIRT]        = SOLID,                   // BLOCK_DIRT
        [BLOCK_GRASS]       = SOLID | TRIGGER,         // BLOCK_GRASS 
        [BLOCK_WOOD]        = SOLID,                   // BLOCK_WOOD
        [BLOCK_TREE]        = SOLID,                   // BLOCK_TREE
        [BLOCK_LEAF]        = SOLID,                   // BLOCK_LEAF
        [BLOCK_LEAF_FLOWER] = SOLID,                   // BLOCK_LEAF_FLOWER
        [BLOCK_WATER]       = WATER | TRIGGER,         // BLOCK_WATER
        [BLOCK_CACTUS]      = SOLID,                   // BLOCK_CACTUS

        [BLOCK_POPPY]       = FOLIAGE,                 // BLOCK_POPPY
        [BLOCK_ORCHID]      = FOLIAGE,                 // BLOCK_ORCHID
        [BLOCK_DANDELION]   = FOLIAGE,                 // BLOCK_DANDELION
        [BLOCK_HERB]        = FOLIAGE,                 // BLOCK_HERB
        [BLOCK_AIR]         = TRIGGER                  // BLOCK_AIR
    };

    inline bool hasProperty(U8 blockType, BlockProperties property) {
        return (BLOCK_PROPERTIES[blockType] & property) != 0;
    }

    inline constexpr U8 cubeFaces[8][4][3] = {
        [DIR_X_FRONT] = {{0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1}},  //drawn clockwise looking x-
        [DIR_X_BACK] =  {{0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}},  //drawn clockwise looking x+
        //[DIR_X_BACK] =  {{1, 1, 1}, {1, 1, 0}, {1, 0, 0}, {1, 0, 1}},  //drawn clockwise looking x+
        
        [DIR_Y_FRONT] = {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},  //drawn clockwise looking y-
        [DIR_Y_BACK] =  {{0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0}},  //drawn clockwise looking y+
        //[DIR_Y_BACK] =  {{0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}},  //drawn clockwise looking y+

        
        [DIR_Z_FRONT] = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}},  //drawn clockwise looking z-
        [DIR_Z_BACK] =  {{1, 1, 0}, {0, 1, 0}, {0, 0, 0}, {1, 0, 0}},  //drawn clockwise looking z+
        //[DIR_Z_BACK] =  {{0, 1, 1}, {1, 1, 1}, {1, 0, 1}, {0, 0, 1}},  //drawn clockwise looking z+

        [DIR_DIAG_XY_FRONT] = {{0, 1, 1}, {1, 1, 0}, {1, 0, 0}, {0, 0, 1}}, //-X+Y+Z To +X-Y-Z 
        [DIR_DIAG_XY_BACK]  = {{0, 1, 0}, {1, 1, 1}, {1, 0, 1}, {0, 0, 0}}  //-X+Y-Z To +X-Y+Z 
    };
    
    inline constexpr S8 cubeNormals[8][3] = {
        [DIR_X_FRONT] = { 1,  0,  0},  // Front (X+)
        [DIR_X_BACK]  = {-1,  0,  0},  // Back (X-)
        [DIR_Y_FRONT] = { 0,  1,  0},  // Top (Y+)
        [DIR_Y_BACK]  = { 0, -1,  0},  // Bottom (Y-)
        [DIR_Z_FRONT] = { 0,  0,  1},  // Left (Z+)
        [DIR_Z_BACK]  = { 0,  0, -1},  // Right (Z-)
        
        [DIR_DIAG_XY_FRONT] ={ 0,  1,  0},  // Normal para la cara superior (Y+)
        [DIR_DIAG_XY_BACK]  ={ 0,  1,  0},  // Normal para la cara superior (Y+)
    };

    struct Transform {
        FVec3 Position;
        FVec3 Rotation;
        FVec3 Scale;

        Transform(cFVec3& pos, cFVec3& rot, cFVec3& sca) : Position(pos), Rotation(rot), Scale(sca) {}
    };

#ifdef OPTIMIZATION_VERTEX_MEMORY
    #ifdef OPTIMIZATION_MAPS
        inline U8 tileUVMap[NUM_TILES][2]{};
    #else
        inline Map<U8, UCVec2> tileUVMap;
    #endif
#else
    #ifdef OPTIMIZATION_MAPS
        inline U16 tileUVMap[NUM_TILES][2]{};
    #else
        inline Map<U8, USVec2> tileUVMap;
    #endif
#endif
    
    using Blocks_Faces = USVec2;
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