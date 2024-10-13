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

#include <locale>

#include <ogc/tpl.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs

#include <fmt/core.h>

//My includes
#include <common.h>

#include <engine.h>
#include <camera.h>
#include <memory.h>

#include <tick.h>
#include <timer.h>

#include <chunk.h>
#include <world.h>
#include <text_renderer.h>

//Blocks TPL data
#include "bloquitos_tpl.h"
#include "bloquitos.h"

// Font
#include <thread>

#include "Karma_ttf.h"

using namespace poyo;

//#define USE_OLD

#define TILE_SIZE 128

static TPLFile bloquitosTPL;
static GXTexObj blocksTexture;
#ifdef OPTIMIZE_MAPS
    static u16 tileUVMap[NUM_TILES][2]{};
#else
    static HashMap<u8, Pair<u16, u16>> tileUVMap;
#endif
static u16 nDrawCalls = 0;

constexpr u16 tileTexCoords[4][2] = {
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1}
};

void mapTileUVs(u8 tilesetWidth) {
#ifdef OPTIMIZE_MAPS
    for (u8 tile = 0; tile < NUM_TILES; tile++) {
        auto U = tile % tilesetWidth;     // Coordenada U (X)
        auto V = tile / tilesetWidth;     // Coordenada V (Y)
        tileUVMap[tile][0] = U;
        tileUVMap[tile][1] = V;
    }
#else
    for (u8 tile = 0; tile < NUM_TILES; tile++) {
        auto U = tile % tilesetWidth;     // Coordenada U (X)
        auto V = tile / tilesetWidth;     // Coordenada V (Y)
        tileUVMap[tile] = {U, V};
    }
#endif
}

double convertirBytesAKilobytes(size_t bytes) {
    const double BYTES_POR_KILO = 1024.0; // 1 KB = 1024 bytes
    return bytes / BYTES_POR_KILO;
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
    GRRLIB_ObjectView(cube.x  + worldX, cube.y  + worldY, cube.z  + worldZ,
        angleX, angleY, 0.0f, 1.0f, 1.0f, 1.0f);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 24);

    for (auto& currentFace : cube.face) {
        for (int j = 0; j < 4; j++) {
            //Signed Short!!
            GX_Position3u16(currentFace.x + cubeFaces[currentFace.direction][j][0],
                            currentFace.y + cubeFaces[currentFace.direction][j][1],
                            currentFace.z + cubeFaces[currentFace.direction][j][2]);
            //GX_Color1u32 old
            //GX_Color1x8(255);
            GX_Color1u32(0xFFFFFFFF);
            //GX_TexCoord2f32 old

            auto& UV = tileUVMap[currentFace.tile];

#ifdef OPTIMIZE_MAPS
            GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
            //GX_TexCoord2f32(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#else
            GX_TexCoord2u16(UV.first + tileTexCoords[j][0], UV.second + tileTexCoords[j][1]);
#endif
        }
    }
    GX_End();
}

void renderChunk(const Chunk& chunk) {
    auto& cubitos = chunk.cubitos_;
#ifdef OPTIMIZE_VECTOR
    for (const auto& currentCubito : cubitos) {
        if(currentCubito.type == BLOCK_AIR) continue;
        nDrawCalls++;

        renderCube(currentCubito, 0, 0, chunk.position_.x, 0, chunk.position_.y);
    }
#else
    for (size_t x = 0; x < cubitos.size(); ++x) {
        for (size_t y = 0; y < cubitos[x].size(); ++y) {
            for (size_t z = 0; z < cubitos[x][y].size(); ++z) {
                const Cubito& currentCubito = cubitos[x][y][z];
                if(currentCubito.type == BLOCK_AIR) continue;
                nDrawCalls++;
                renderCube(currentCubito, 0, 0, chunk.position_.x, 0, chunk.position_.y);
            }
        }
    }
#endif
}

String formatThousands(size_t value) {
    String num_str = std::to_string(value);  // Convert the number to a string
    auto insert_position = num_str.length() - 3;   // Start 3 digits from the end

    // Insert commas every three digits
    while (insert_position > 0) {
        num_str.insert(insert_position, ".");
        insert_position -= 3;
    }

    return num_str;
}

int main(int argc, char **argv) {
    size_t used1 = Memory::getTotalMemoryUsed();
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

    mapTileUVs(6);

#ifdef USE_OLD
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

#endif
    
    TextRenderer text;
    Camera currentCam(FVec3{0.0f, 20.0f, 50.0f}, -20, -90, 15.0f);
    
    size_t used2 = Memory::getTotalMemoryUsed();
    
    World currentWorld;

    auto& currentChunk = currentWorld.getOrCreateChunk(0, 0);
    currentWorld.generateChunk(currentChunk, 0, 0);
    auto& currentChunk2 = currentWorld.getOrCreateChunk(CHUNK_SIZE, 0);
    currentWorld.generateChunk(currentChunk2, CHUNK_SIZE, 0);
    
    size_t used3 = Memory::getTotalMemoryUsed();

    Tick currentTick;
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
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_U16, 0); //Positions -> U16
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0); //Textures  -> U16
        
#ifdef USE_OLD
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
#endif

        nDrawCalls = 0;
        currentTick.start();
        renderChunk(currentChunk);
        renderChunk(currentChunk2);
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        auto drawTicks = currentTick.stopAndGetTick();
        currentTick.reset();
        
        if(PAD_ButtonsHeld(0) & PAD_TRIGGER_Z) currentCam.setPosition(FVec3{0, 0, 0});
        

        auto camPos = currentCam.getPosition();
        GRRLIB_2dMode();
        
        text.beginRender();
        text.render(USVec2{5,   5}, fmt::format("Ticks (CPU) : {}", gettick()).c_str());
        text.render(USVec2{5,  20}, fmt::format("Time        : {}", gettime()).c_str());
        text.render(USVec2{5,  35}, fmt::format("Current Time: {}", Engine::getCurrentTime()).c_str());
        text.render(USVec2{5,  50}, fmt::format("Last Time   : {}", Engine::getLastTime()).c_str());
        text.render(USVec2{5,  65}, fmt::format("Delta Time  : {:.3f} s", deltaTime).c_str());
        text.render(USVec2{5,  80}, fmt::format("Mem1  : {}", used1).c_str());
        text.render(USVec2{5,  95}, fmt::format("Mem2  : {}", used2).c_str());
        text.render(USVec2{5,  110}, fmt::format("Mem3  : {}", used3).c_str());
        text.render(USVec2{5,  125}, fmt::format("Mem  : {:.2f} KB", convertirBytesAKilobytes(used3 - used2)).c_str());
        text.render(USVec2{5,  140}, fmt::format("Free Memory  : {}", SYS_GetArena1Size()).c_str());
        text.render(USVec2{275,  5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
        text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());
        //Render Things
        text.render(USVec2{400,  50}, fmt::format("NDraw Calls : {}", nDrawCalls).c_str());
        text.render(USVec2{400,  65}, fmt::format("Draw Cycles : {} ts", formatThousands(drawTicks)).c_str());
        text.render(USVec2{400,  80}, fmt::format("Draw Time   : {} ms", Tick::TickToMs(drawTicks)).c_str());
        text.render(USVec2{400,  95}, fmt::format("Helper      : {}", currentWorld.helperCounter).c_str());
        text.render(USVec2{400, 110}, fmt::format("N Blocks    : {}", currentChunk.validBlocks).c_str());

        //GRRLIB_PrintfTTF(50, 50, myFont, "MINECRAFT", 16, 0x000000FF);

        // Renderizar todo a la pantalla
        GRRLIB_Render(); // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

