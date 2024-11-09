/*===========================================
GRRLIB (GX Version)
        - Template Code -

        Minimum Code To Use GRRLIB
============================================*/
//#include <grrlib.h>
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
#include <utilities.h>

#include <engine.h>
#include <camera.h>
#include <renderer.h>
#include <memory.h>

#include <tick.h>
#include <timer.h>

#include <chunk.h>
#include <world.h>
#include <text_renderer.h>

//TPL and Fonts
#include <main.h>

using namespace poyo;

#define TILE_SIZE 128

static u16 nDrawCalls = 0;
guVector lightPos{0, 10, 0};

struct Options {
    bool boundingBox = false;
    bool lightning = false;
    bool debugUI = false;
    bool VSYNC = true;
};

Options options;

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
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 3, baseZ + j, BLOCK_LEAF2);

            // Nivel de hojas en y + 6
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 6, baseZ + j, BLOCK_LEAF2);
        }
    }

    // Generar las hojas en los niveles intermedios (y + 4 y y + 5)
    for (s16 i = -2; i <= 2; i++) {
        for (s16 j = -2; j <= 2; j++) {
            // Nivel de hojas en y + 4
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 4, baseZ + j, BLOCK_LEAF2);

            // Nivel de hojas en y + 5
            generateCube(cubes[cubeIndex++], baseX + i, baseY + 5, baseZ + j, BLOCK_LEAF2);
        }
    }

    // Generar el tronco del árbol
    for (s16 i = 0; i < 4; i++) {
        generateCube(cubes[cubeIndex++], baseX, baseY + i, baseZ, BLOCK_TREE);
    }
}

void updatePosition(guVector& point, float radius, float angle) {
    // Actualiza las coordenadas X y Z para que el punto se mueva en un círculo
    point.x = radius * cos(angle); // Movimiento circular en el eje X
    point.z = radius * sin(angle); // Movimiento circular en el eje Z
}

//TODO: El lesson 8 tiene transparencias
//Lesson 9 una luz toh guapa
//lesson 10 tiene movimiento bien
//display list en CavEX
//Compilar con O3

int main(int argc, char **argv) {
    float angle = 0.0f;
    size_t used1 = Memory::getTotalMemoryUsed();
    // Initialise the Graphics & Video subsystem
    //-----GRRLIB_Init();
    Renderer::InitializeGX();
   
    // Initialise the GameCube controllers
    PAD_Init();

    //GRRLIB_SetAntiAliasing(true);

    loadResources();

    // Set the background color (Green in this case)
    //-----GRRLIB_SetBackgroundColour(0x80, 0x80, 0x80, 0xFF);
    Renderer::SetBackgroundColour(0x80, 0x80, 0x80, 0xFF);
    //-----GRRLIB_SetLightAmbient(0x000000FF); //0x333333FF

    Renderer::Initialize();

    Cubito Tree1[72];

    generateTree(Tree1, 0, 1, 0);
    
    TextRenderer text;
    float CameraSpeed = 15.0f;
    Camera currentCam(FVec3{0.0f, 30.0f, 50.0f}, -20, -90, CameraSpeed);
    
    size_t used2 = Memory::getTotalMemoryUsed();
    
    World currentWorld;

    // Definir el número de chunks a generar en ambas direcciones
    //S16 numChunksX = 2; // Número de chunks a generar en la dirección X
    //S16 numChunksZ = 2; // Número de chunks a generar en la dirección Z
    
    //currentWorld.generateChunks(0, 0, numChunksX, numChunksZ);
    SYS_Report("N Blocks: %llu\n", 0);
    currentWorld.generateLand(CHUNK_RADIUS);
    SYS_Report("N Blocks: %llu\n", currentWorld.validBlocks_);
    //SYS_Report("Start X Z: %zd %zd\n", startX, startZ);
    
    size_t used3 = Memory::getTotalMemoryUsed();

    Tick currentTick;
    
    Cubito deleteMe;
    generateCube(deleteMe, 0,  10, 0, BLOCK_LEAF2);
    //Start from the first GX command after VSync, and end after GX_DrawDone().
    // GX_SetDrawDone();
    // Loop forever
    while(1) {
        Engine::UpdateEngine();
        auto deltaTime = Engine::getDeltaTime();
        Renderer::ResetDrawCalls();
        
        if(options.lightning) {
            angle+=deltaTime;
            updatePosition(lightPos, 20, angle);
        }

        
        //-----GRRLIB_2dMode();
        Renderer::Set2DMode();
        PAD_ScanPads(); // Scan the GameCube controllers

        // If [START/PAUSE] was pressed on the first GameCube controller, break out of the loop
        if(PAD_ButtonsDown(0) & PAD_BUTTON_START) break;
        if(PAD_ButtonsDown(0) & PAD_TRIGGER_Z) currentCam.setPosition(FVec3(0));
        if(PAD_ButtonsDown(0) & PAD_BUTTON_UP) options.debugUI = !options.debugUI;;
        if(PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT) options.boundingBox = !options.boundingBox;;
        if(PAD_ButtonsDown(0) & PAD_BUTTON_LEFT) options.lightning = !options.lightning;
        if(PAD_ButtonsDown(0) & PAD_BUTTON_DOWN) options.VSYNC = !options.VSYNC;
        if(PAD_ButtonsDown(0) & PAD_TRIGGER_R) currentCam.setSpeed(CameraSpeed * 5.0f);
        if(PAD_ButtonsUp(0) & PAD_TRIGGER_R) currentCam.setSpeed(CameraSpeed);
        
        currentCam.updateCamera(deltaTime); //deltaTime
        //-----
        // GRRLIB_3dMode(0.1f, 1000.0f, 45.0f, false, true); // Configura el modo 3D //Projection
        Renderer::Set3DMode(currentCam); // Configura el modo 3D //Projection
        // GRRLIB_SetLightOff();
        // GRRLIB_ObjectView(lightPos.x, lightPos.y, lightPos.z, 0,0,0,1,1,1);
        // GRRLIB_DrawSphere(1, 20, 20, true, 0xFFFF00FF);
        // if(options.lightning) {
        //     GRRLIB_SetLightDiff(1, lightPos,20.0f,1.0f,0xFFFFFFFF);
        // }
        //
        // GRRLIB_ObjectView(1.0f,20,0, 0,0,0,1,1,1);
        // GRRLIB_DrawCube(1, true, 0xFFFFFFFF);
        // GRRLIB_ObjectView(0.0f,20,0, 0,0,0,1,1,1);
        // GRRLIB_DrawCube(1, true, 0xFFFFFFFF);

        // Limpiar pantalla y preparar para dibujo en 3D
        //GRRLIB_3dMode(0.1f, 1000.0f, 45.0f, 1, 1); // Configura el modo 3D //Projection
        
        Renderer::BindTexture(blocksTexture, 0);
        Renderer::SetTextureCoordScaling(0, TILE_SIZE, TILE_SIZE);
        Renderer::DisableBlend();
        

        nDrawCalls = 0;
        currentTick.start();
        Renderer::PrepareToRender(true, true, true, true);
        auto& chunkitos = currentWorld.getChunks();
        for(auto& chunkito : chunkitos) {
            chunkito->render();
        }
        
        //currentWorld.renderChunksAround(currentCam.getPosition().x, currentCam.getPosition().z);
        
        if(options.boundingBox) {
            //-----GRRLIB_SetLightOff();
            Renderer::PrepareToRender(true, false, true, false);
            for(const auto& chunkito : chunkitos) {
                Renderer::RenderBoundingBox(chunkito->worldPosition_.x, 0, chunkito->worldPosition_.z, CHUNK_SIZE, UCVec3{0, 255, 255}, true);
            } 
        }
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        auto drawTicks = currentTick.stopAndGetTick();
        currentTick.reset();

        
        auto camPos = currentCam.getPosition();
        // Switch to 2D Mode to display text
        //-----GRRLIB_2dMode();
        if(options.debugUI) {
            Renderer::Set2DMode();
            text.beginRender();
            text.render(USVec2{5,   5}, fmt::format("Ticks (CPU) : {}", gettick()).c_str());
            text.render(USVec2{5,  20}, fmt::format("Time        : {}", gettime()).c_str());
            text.render(USVec2{5,  35}, fmt::format("Current Time: {}", Engine::getCurrentTime()).c_str());
            text.render(USVec2{5,  50}, fmt::format("Last Time   : {}", Engine::getLastTime()).c_str());
            text.render(USVec2{5,  65}, fmt::format("Delta Time  : {:.3f} s", deltaTime).c_str());
            text.render(USVec2{5,  80}, fmt::format("Mem1  : {}", used1).c_str());
            text.render(USVec2{5,  95}, fmt::format("Mem2  : {}", used2).c_str());
            text.render(USVec2{5,  110}, fmt::format("Mem3  : {}", used3).c_str());
            text.render(USVec2{5,  125}, fmt::format("Mem  : {:.2f} KB", convertBytesToKilobytes(used3 - used2)).c_str());
            text.render(USVec2{5,  140}, fmt::format("Free Memory  : {:.2f} KB", convertBytesToKilobytes(SYS_GetArena1Size())).c_str());
            text.render(USVec2{275,  5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
            text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());
            //Render Things

            text.render(USVec2{400,  35}, fmt::format("Valid Blocks : {}", currentWorld.validBlocks_).c_str());
            text.render(USVec2{400,  50}, fmt::format("NDraw Calls : {}", Renderer::DrawCalls()).c_str());
            text.render(USVec2{400,  65}, fmt::format("NFaces Drawn : {}", Renderer::FacesDrawn()).c_str());
            // text.render(USVec2{400,  65}, fmt::format("Draw Cycles : {} ts", formatThousands(drawTicks)).c_str());
            // text.render(USVec2{400,  80}, fmt::format("Draw Time   : {} ms", Tick::TickToMs(drawTicks)).c_str());
            //text.render(USVec2{400,  95}, fmt::format("Helper      : {}", currentWorld.helperCounter).c_str());
            //text.render(USVec2{400, 110}, fmt::format("N Blocks    : {}", currentChunk.validBlocks).c_str());
            text.render(USVec2{400, 110}, fmt::format("Video Mode    : {}", VIDEO_GetVideoScanMode()).c_str());
            text.render(USVec2{400, 125}, fmt::format("VSYNC    : {}", options.VSYNC ? "YES" : "NOP").c_str());
        }


        //GRRLIB_PrintfTTF(50, 50, myFont, "MINECRAFT", 16, 0x000000FF);

        // Renderizar todo a la pantalla
        //-----GRRLIB_Render(); // Render the frame buffer to the TV
        Renderer::RenderGX(options.VSYNC);
    }

    //-----GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

