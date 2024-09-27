/*===========================================
GRRLIB (GX Version)
        - Template Code -

        Minimum Code To Use GRRLIB
============================================*/
#include <grrlib.h>
#include <stdlib.h>
#include <ogc/pad.h>
#include <math.h> // Para usar funciones trigonométricas

#include <unordered_map>

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>

#include <ogc/tpl.h>

//Blocks TPL data
#include "bloquitos_tpl.h"
#include "bloquitos.h"

#define TILE_SIZE 128

static TPLFile bloquitosTPL;
static GXTexObj blocksTexture;
static std::unordered_map<u8, std::pair<u16, u16>> tileUVMap;

struct CubeFace {
    s16 x, y, z;
    u8 direction;
    u8 tile;
};

struct Cubito {
    CubeFace face[6];
    s16 x, y, z;
};

enum {
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_DIRT,
    BLOCK_GRASS,
    BLOCK_WOOD,
    BLOCK_TREE,
    BLOCK_LEAF,
    BLOCK_GAMECUBE,
    BLOCK_COUNT // Contador para la cantidad de bloques
};

enum {
    TILE_DIRT       = 0,
    TILE_GRASS_SIDE = 1,
    TILE_GRASS      = 2,

    TILE_STONE      = 3,

    TILE_SAND       = 6,
    TILE_TREE_SIDE  = 9,
    TILE_TREE_TOP   = 10,
    TILE_WOOD       = 11,
    TILE_LEAF       = 12,
    NUM_TILES,
};

enum {
    DIR_X_FRONT,
    DIR_X_BACK,
    DIR_Y_FRONT,
    DIR_Y_BACK,
    DIR_Z_FRONT,
    DIR_Z_BACK,
};

static const u8 blockTiles[][6] = {
    [BLOCK_STONE] =    {TILE_STONE,      TILE_STONE,      TILE_STONE,    TILE_STONE,     TILE_STONE,      TILE_STONE},
    [BLOCK_SAND] =     {TILE_SAND,       TILE_SAND,       TILE_SAND,     TILE_SAND,      TILE_SAND,       TILE_SAND},
    [BLOCK_DIRT] =     {TILE_DIRT,       TILE_DIRT,       TILE_DIRT,     TILE_DIRT,      TILE_DIRT,       TILE_DIRT},
    [BLOCK_GRASS] =    {TILE_GRASS_SIDE, TILE_GRASS_SIDE, TILE_GRASS,    TILE_DIRT,      TILE_GRASS_SIDE, TILE_GRASS_SIDE},
    [BLOCK_WOOD] =     {TILE_WOOD,       TILE_WOOD,       TILE_WOOD,     TILE_WOOD,      TILE_WOOD,       TILE_WOOD},
    [BLOCK_TREE] =     {TILE_TREE_SIDE,  TILE_TREE_SIDE,  TILE_TREE_TOP, TILE_TREE_TOP,  TILE_TREE_SIDE,  TILE_TREE_SIDE},
    [BLOCK_LEAF] =     {TILE_LEAF,       TILE_LEAF,       TILE_LEAF,     TILE_LEAF,      TILE_LEAF,       TILE_LEAF}//,
};


static const u8 cubeFaces[6][4][3] = {
    [DIR_X_FRONT] = {{0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1}},  //drawn clockwise looking x-
    [DIR_X_BACK] =  {{0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}},  //drawn clockwise looking x+
    [DIR_Y_FRONT] = {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},  //drawn clockwise looking y-
    [DIR_Y_BACK] =  {{0, 0, 0}, {0, 0, 1}, {1, 0, 1}, {1, 0, 0}},  //drawn clockwise looking y+
    [DIR_Z_FRONT] = {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}},  //drawn clockwise looking z-
    [DIR_Z_BACK] =  {{1, 1, 0}, {0, 1, 0}, {0, 0, 0}, {1, 0, 0}},  //drawn clockwise looking z+
};

void mapTileUVs(u8 tilesetWidth) {
    for (u8 tile = 0; tile < NUM_TILES; tile++) {
        auto U = tile % tilesetWidth;     // Coordenada U (X)
        auto V = tile / tilesetWidth;     // Coordenada V (Y)
        tileUVMap[tile] = {U, V};
    }
}

void fillCube(Cubito& cube, int cface, int x, int y, int z, int direction, int block) {
    cube.face[cface].x = x;
    cube.face[cface].y = y;
    cube.face[cface].z = z;
    cube.face[cface].direction = direction;
    cube.face[cface].tile = blockTiles[block][direction];
}

void generateCube(Cubito& cube, s16 x, s16 y, s16 z, int block) {
    cube.x = x;
    cube.y = y;
    cube.z = z;
    fillCube(cube, 0, 1, 0, 0, DIR_X_FRONT, block);  // Frente
    fillCube(cube, 1, 0, 0, 0, DIR_X_BACK,  block);  // Detrás
    fillCube(cube, 2, 0, 1, 0, DIR_Y_FRONT, block);  // Arriba
    fillCube(cube, 3, 0, 0, 0, DIR_Y_BACK,  block);  // Abajo well
    fillCube(cube, 4, 0, 0, 1, DIR_Z_FRONT, block);  // Izquierda
    fillCube(cube, 5, 0, 0, 0, DIR_Z_BACK,  block);  // Derecha
}

void renderCube(Cubito& cube, float angleX, float angleY) {
    GRRLIB_ObjectView(cube.x, cube.y, cube.z, angleX, angleY, 0.0f, 1.0f, 1.0f, 1.0f);
    const u16 tileTexCoords[4][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    };

    GX_Begin(GX_QUADS, GX_VTXFMT0, 24);

    int x = 0;
    int z = 0;

    for (int face = 0; face < 6; face++) {
        auto& currentFace = cube.face[face];
        for (int j = 0; j < 4; j++) {
            //Signed Short!!
            GX_Position3s16(currentFace.x + cubeFaces[currentFace.direction][j][0] + x,
                            currentFace.y + cubeFaces[currentFace.direction][j][1],
                            currentFace.z + cubeFaces[currentFace.direction][j][2] + z);
            //GX_Color1u32 old
            //GX_Color1x8(255);
            GX_Color1u32(0xFFFFFFFF);
            //GX_TexCoord2f32 old

            auto UV = tileUVMap[currentFace.tile];

            GX_TexCoord2f32(UV.first + tileTexCoords[j][0], UV.second + tileTexCoords[j][1]);
        }
    }


    GX_End();
}


int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the GameCube controllers
    PAD_Init();

    GRRLIB_SetAntiAliasing(true);

    TPL_OpenTPLFromMemory(&bloquitosTPL, (void*)bloquitos_tpl, bloquitos_tpl_size);
    TPL_GetTexture(&bloquitosTPL, blocksTextureId, &blocksTexture);
    GX_InitTexObjFilterMode(&blocksTexture, GX_NEAR, GX_NEAR);
    GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
    GX_InvalidateTexAll();

    // Set the background color (Green in this case)
    GRRLIB_SetBackgroundColour(0x80, 0x80, 0x80, 0xFF);
    GRRLIB_Camera3dSettings(0.0f,0.0f,13.0f, 0,1,0, 0,0,0);

    float angleX = 45.0f; // Ángulo de rotación en el eje X
    float angleY = 0.0f; // Ángulo de rotación en el eje Y
    //float cubeSize = 2.0f; // Tamaño del cubo

    mapTileUVs(6);

    Cubito stone;
    Cubito grass;
    Cubito trunk;

    generateCube(stone, 0, -2, 0, BLOCK_STONE);
    generateCube(grass, 0,  0, 0, BLOCK_GRASS);
    generateCube(trunk, 0,  2, 0, BLOCK_TREE);

    // Loop forever
    while(1) {
        GRRLIB_2dMode();
        PAD_ScanPads(); // Scan the GameCube controllers

        // If [START/PAUSE] was pressed on the first GameCube controller, break out of the loop
        if(PAD_ButtonsDown(0) & PAD_BUTTON_START) exit(0);

        // Limpiar pantalla y preparar para dibujo en 3D
        GRRLIB_3dMode(0.1f, 1000.0f, 45.0f, 1, 0); // Configura el modo 3D


        //GRRLIB_DrawCube(cubeSize, true, 0xFFFFFFFF); // Dibuja un cubo blanco
        //GRRLIB_SetTexture(tex_girl, 0);
        GX_LoadTexObj(&blocksTexture, GX_TEXMAP0);
        GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, TILE_SIZE, TILE_SIZE);

        
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);

        // Dibujar cubo en el centro de la pantalla con rotación
        renderCube(stone, angleX, angleY);
        renderCube(grass, angleX, angleY);
        renderCube(trunk, angleX, angleY);

        // Incrementar los ángulos de rotación para darle animación
        //angleX += 0.5f; // Rotación en el eje X
        angleY += 0.5f; // Rotación en el eje Y

        // Renderizar todo a la pantalla
        GRRLIB_Render(); // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

//void drawCube() {
//    GX_Begin(GX_QUADS, GX_VTXFMT0, 24);
//    GX_Position3f32(-1.0f, 1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 0.0f);
//    GX_Position3f32(1.0f, 1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 0.0f);
//    GX_Position3f32(1.0f, -1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 1.0f);
//    GX_Position3f32(-1.0f, -1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 1.0f);
//
//    GX_Position3f32(1.0f, 1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 0.0f);
//    GX_Position3f32(-1.0f, 1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 0.0f);
//    GX_Position3f32(-1.0f, -1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 1.0f);
//    GX_Position3f32(1.0f, -1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 1.0f);
//
//    GX_Position3f32(1.0f, 1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 0.0f);
//    GX_Position3f32(1.0f, 1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 0.0f);
//    GX_Position3f32(1.0f, -1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 1.0f);
//    GX_Position3f32(1.0f, -1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 1.0f);
//
//    GX_Position3f32(-1.0f, 1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 0.0f);
//    GX_Position3f32(-1.0f, 1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 0.0f);
//    GX_Position3f32(-1.0f, -1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 1.0f);
//    GX_Position3f32(-1.0f, -1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 1.0f);
//
//    GX_Position3f32(-1.0f, 1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 0.0f);
//    GX_Position3f32(1.0f, 1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 0.0f);
//    GX_Position3f32(1.0f, 1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 1.0f);
//    GX_Position3f32(-1.0f, 1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 1.0f);
//
//    GX_Position3f32(1.0f, -1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 0.0f);
//    GX_Position3f32(-1.0f, -1.0f, -1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 0.0f);
//    GX_Position3f32(-1.0f, -1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(1.0f, 1.0f);
//    GX_Position3f32(1.0f, -1.0f, 1.0f);
//    GX_Color1u32(0xFFFFFFFF);
//    GX_TexCoord2f32(0.0f, 1.0f);
//    GX_End();
//}

