/*===========================================
GRRLIB (GX Version)
        - Template Code -

        Minimum Code To Use GRRLIB
============================================*/
#include <grrlib.h>
#include <cstdlib>
#include <ogc/pad.h>
#include <math.h> // Para usar funciones trigonométricas
#include <gccore.h>

#include <ogc/tpl.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs

#include <fmt/core.h>

//My includes
#include <typedefs.h>
#include <engine.h>
#include <camera.h>
#include <text_renderer.h>

//Blocks TPL data
#include "bloquitos_tpl.h"
#include "bloquitos.h"

// Font
#include "Karma_ttf.h"

using namespace poyo;

#define TILE_SIZE 128

static TPLFile bloquitosTPL;
static GXTexObj blocksTexture;
static HashMap<u8, Pair<u16, u16>> tileUVMap;

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
    BLOCK_CACTUS,
    BLOCK_SAND_DIRT,
    BLOCK_COUNT // Contador para la cantidad de bloques
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
    [BLOCK_LEAF] =     {TILE_LEAF,       TILE_LEAF,       TILE_LEAF,     TILE_LEAF,      TILE_LEAF,       TILE_LEAF},
    [BLOCK_CACTUS] =   {TILE_CACTUS_SIDE, TILE_CACTUS_SIDE, TILE_CACTUS_TOP, TILE_CACTUS_BOT, TILE_CACTUS_SIDE, TILE_CACTUS_SIDE},
    [BLOCK_SAND_DIRT] ={TILE_SAND_DIRT, TILE_SAND_DIRT, TILE_SAND, TILE_DIRT, TILE_SAND_DIRT, TILE_SAND_DIRT},
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

void fillCube(Cubito& cube, u16 face, s16 x, s16 y, s16 z, u8 direction, int block) {
    cube.face[face].x = x;
    cube.face[face].y = y;
    cube.face[face].z = z;
    cube.face[face].direction = direction;
    cube.face[face].tile = blockTiles[block][direction];
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

void generateTree(Cubito cubes[], s16 baseX, s16 baseY, s16 baseZ) {
    // Generar las hojas en las coordenadas superiores
    int cubeIndex = 0;
    for (s16 i = -1; i <= 1; i++) {
        for (s16 j = -1; j <= 1; j++) {
            // Nivel de hojas en y + 3
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 3, baseZ + j, BLOCK_LEAF);

            // Nivel de hojas en y + 6
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 6, baseZ + j, BLOCK_LEAF);
        }
    }

    // Generar las hojas en los niveles intermedios (y + 4 y y + 5)
    for (s16 i = -2; i <= 2; i++) {
        for (s16 j = -2; j <= 2; j++) {
            // Nivel de hojas en y + 4
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 4, baseZ + j, BLOCK_LEAF);

            // Nivel de hojas en y + 5
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 5, baseZ + j, BLOCK_LEAF);
        }
    }

    // Generar el tronco del árbol
    for (s16 i = 0; i < 4; i++) {
        generateCube(cubes[cubeIndex++], baseX, baseY + i, baseZ, BLOCK_TREE);
    }
}

void renderCube(const Cubito& cube, float angleX, float angleY, s16 worldX = 0, s16 worldY = 0, s16 worldZ = 0) {
    GRRLIB_ObjectView(cube.x, cube.y, cube.z, angleX, angleY, 0.0f, 1.0f, 1.0f, 1.0f);
    const u16 tileTexCoords[4][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    };

    GX_Begin(GX_QUADS, GX_VTXFMT0, 24);

    for (int face = 0; face < 6; face++) {
        auto& currentFace = cube.face[face];
        for (int j = 0; j < 4; j++) {
            //Signed Short!!
            GX_Position3s16(currentFace.x + cubeFaces[currentFace.direction][j][0] + worldX,
                            currentFace.y + cubeFaces[currentFace.direction][j][1] + worldY,
                            currentFace.z + cubeFaces[currentFace.direction][j][2] + worldZ);
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
    GRRLIB_Settings.antialias = true;
    
    //GRRLIB_ttfFont *myFont = GRRLIB_LoadTTF(Karma_ttf, Karma_ttf_size);

    TPL_OpenTPLFromMemory(&bloquitosTPL, (void*)bloquitos_tpl, bloquitos_tpl_size);
    TPL_GetTexture(&bloquitosTPL, blocksTextureId, &blocksTexture);
    GX_InitTexObjFilterMode(&blocksTexture, GX_NEAR, GX_NEAR);
    GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
    GX_InvalidateTexAll();

    // Set the background color (Green in this case)
    GRRLIB_SetBackgroundColour(0x80, 0x80, 0x80, 0xFF);
    //GRRLIB_Camera3dSettings(0.0f,0.0f,13.0f, 0,1,0, 0,0,0); //view matrix

    float angleX = 45.0f; // Ángulo de rotación en el eje X
    float angleY = 0.0f; // Ángulo de rotación en el eje Y
    //float cubeSize = 2.0f; // Tamaño del cubo

    mapTileUVs(6);

    Cubito grass[9];
    Cubito stone[25];
    Cubito dirt[25];
    Cubito cactus[3];

    Cubito sandDirt[9];

    //Cubito Tree1[26];
    Cubito Tree1[72];

    generateTree(Tree1, 0, 1, 0);
    
    int cubeIndex = 0;
    for (s16 i = -1; i <= 1; i++) {       // Recorre las posiciones -1, 0, 1 en el eje X
        for (s16 j = -1; j <= 1; j++) {   // Recorre las posiciones -1, 0, 1 en el eje Z
            generateCube(grass[cubeIndex], i, 0, j, BLOCK_GRASS);  // Genera el bloque en la posición (x, y, z)
            cubeIndex++;  // Aumenta el índice para el siguiente cubo
        }
    }
    cubeIndex = 0;
    for (s16 i = -2; i <= 2; i++) {       // Recorre las posiciones -2, -1, 0, 1, 2 en el eje X
        for (s16 j = -2; j <= 2; j++) {   // Recorre las posiciones -2, -1, 0, 1, 2 en el eje Z
            generateCube(stone[cubeIndex], i, -1, j, BLOCK_STONE);  // Genera el bloque en la posición (x, y, z)
            cubeIndex++;  // Aumenta el índice para el siguiente cubo
        }
    }
    cubeIndex = 0;

    //Cactus:
    int baseX = -6;  // Posición inicial en el eje X
    int baseY = 0;   // Posición en el eje Y (constante en este caso)
    int baseZ = 0;   // Posición inicial en el eje Z
    
    generateCube(cactus[0], -6,  1, 0, BLOCK_CACTUS);
    generateCube(cactus[1], -6,  2, 0, BLOCK_CACTUS);
    generateCube(cactus[2], -6,  3, 0, BLOCK_CACTUS);

    // Iterar para crear una cuadrícula de 3x3 alrededor de (-6, 0, 0)
    for (s16 i = -1; i <= 1; i++) {       // Recorre las posiciones -1, 0, 1 en el eje X
        for (s16 j = -1; j <= 1; j++) {   // Recorre las posiciones -1, 0, 1 en el eje Z
            generateCube(sandDirt[cubeIndex], baseX + i, baseY, baseZ + j, BLOCK_SAND_DIRT);  // Genera el bloque en la posición (x, y, z)
            cubeIndex++;  // Aumenta el índice para el siguiente cubo
        }
    }
    cubeIndex = 0;
    for (s16 i = -2; i <= 2; i++) {       // Recorre las posiciones -2, -1, 0, 1, 2 en el eje X
        for (s16 j = -2; j <= 2; j++) {   // Recorre las posiciones -2, -1, 0, 1, 2 en el eje Z
            generateCube(dirt[cubeIndex], baseX + i, -1, j, BLOCK_DIRT);  // Genera el bloque en la posición (x, y, z)
            cubeIndex++;  // Aumenta el índice para el siguiente cubo
        }
    }
    
    TextRenderer text;
    Camera currentCam(FVec3{0.0f, 0.0f, 13.0f}, 15.0f);
    
    // Loop forever
    while(1) {
        Engine::UpdateEngine();
        auto deltaTime = Engine::getDeltaTime();
        
        GRRLIB_2dMode();
        PAD_ScanPads(); // Scan the GameCube controllers

        // If [START/PAUSE] was pressed on the first GameCube controller, break out of the loop
        if(PAD_ButtonsDown(0) & PAD_BUTTON_START) exit(0);

        currentCam.updateCamera(deltaTime); //deltaTime
        // Limpiar pantalla y preparar para dibujo en 3D
        GRRLIB_3dMode(0.1f, 1000.0f, 45.0f, 1, 0); // Configura el modo 3D //Projection

        //GRRLIB_DrawCube(cubeSize, true, 0xFFFFFFFF); // Dibuja un cubo blanco
        //GRRLIB_SetTexture(tex_girl, 0);
        GX_LoadTexObj(&blocksTexture, GX_TEXMAP0);
        GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, TILE_SIZE, TILE_SIZE);
        
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);

        // Dibujar cubo en el centro de la pantalla con rotación
        // renderCube(stone, angleX, angleY);
        // renderCube(grass, angleX, angleY);
        // renderCube(trunk, angleX, angleY);

        for(int i = 0; i < 9; i++) {
            renderCube(grass[i], 0, 0, 0, 0, 0);
        }
        for (auto& i : stone) {
            renderCube(i, 0, 0, 0, 0, 0);
        }
        for (int i = 0; i < 72; i++) {
            renderCube(Tree1[i], 0, 0, 0, 0, 0);
        }

        //Cactus
        for (auto& cactu : cactus) {
            renderCube(cactu, 0, 0);
        }
        for (auto& i : sandDirt) {
            renderCube(i, 0, 0);
        }
        for (auto& i : dirt) {
            renderCube(i, 0, 0);
        }

        if(PAD_ButtonsHeld(0) & PAD_TRIGGER_Z) currentCam.setPosition(FVec3{0, 0, 0});
        

        auto camPos = currentCam.getPosition();
        GRRLIB_2dMode();
        
        text.beginRender();
        text.render(USVec2{5,   5}, fmt::format("Ticks (CPU) : {}", gettick()).c_str());
        text.render(USVec2{5,  20}, fmt::format("Time        : {}", gettime()).c_str());
        text.render(USVec2{5,  35}, fmt::format("Current Time: {}", Engine::getCurrentTime()).c_str());
        text.render(USVec2{5,  50}, fmt::format("Last Time   : {}", Engine::getLastTime()).c_str());
        text.render(USVec2{5,  65}, fmt::format("Delta Time  : {}", deltaTime).c_str());
        text.render(USVec2{275, 5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
        text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());

        //GRRLIB_PrintfTTF(50, 50, myFont, "MINECRAFT", 16, 0x000000FF);

        // Incrementar los ángulos de rotación para darle animación
        //angleX += 0.5f; // Rotación en el eje X
        angleY += 0.5f; // Rotación en el eje Y

        // Renderizar todo a la pantalla
        GRRLIB_Render(); // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

