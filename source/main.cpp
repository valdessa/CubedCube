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

#define TILE_SIZE 16

static u16 nDrawCalls = 0;

struct Options {
    bool boundingBox = false;
    bool lightning = false;
    bool debugUI = false;
    bool VSYNC = true;
};

void updatePosition(FVec3& point, float radius, float angle) {
    // Actualiza las coordenadas X y Z para que el punto se mueva en un círculo
    point.x = radius * cos(angle); // Movimiento circular en el eje X
    point.z = radius * sin(angle); // Movimiento circular en el eje Z
}

//TODO: El lesson 8 tiene transparencias
//Lesson 9 una luz toh guapa
//lesson 10 tiene movimiento bien
//display list en CavEX
//Compilar con O3

inline constexpr U8 grassQuadFaces[2][4][3] = {
    // Primer quad (diagonal de esquina superior izquierda a inferior derecha)
    {{0, 1, 1}, {1, 1, 0}, {1, 0, 0}, {0, 0, 1}},  // Quad 1

    // Segundo quad (diagonal de esquina superior derecha a inferior izquierda)
    {{0, 1, 0}, {1, 1, 1}, {1, 0, 1}, {0, 0, 0}},  // Quad 2
};

inline constexpr S8 grassNormals[2][3] = {
    { 0,  1,  0},  // Normal para la cara superior (Y+)
    { 0,  1,  0},  // Normal para la cara superior (Y+)
};

int main(int argc, char **argv) {
    // size_t MemoryUsedAtbeginning = (uintptr_t)SYS_GetArena1Lo() - (uintptr_t)SYS_GetArena1Hi;
    // MemoryUsedAtbeginning += SYS_GetArena1Size();
    Options options;
    //srand(time(nullptr));
    float angle = 0.0f;
    size_t MemoryUsedBySystem = Memory::getTotalMemoryUsed();
    
    // Initialise the Graphics & Video subsystem
    Renderer::InitializeGX();

    MemoryUsedBySystem = Memory::getTotalMemoryUsed() - MemoryUsedBySystem;
   
    // Initialise the GameCube controllers
    PAD_Init();

    loadResources();

    // Set the background color (Green in this case)
    Renderer::SetBackgroundColour(0x80, 0x80, 0x80, 0xFF);
    Renderer::SetLightAmbient(0, 0, 0, 255);

    Renderer::Initialize();
    
    TextRenderer text;
    float CameraSpeed = 15.0f;
    Camera currentCam(FVec3{0.0f, 30.0f, 50.0f}, -20, -90, CameraSpeed);


    auto MemoryUsedByVoxel = Memory::getTotalMemoryUsed();
    GX_SetZCompLoc(GX_FALSE);
    World currentWorld;
    
    SYS_Report("N Blocks: %llu\n", 0);
    currentWorld.generateLand(CHUNK_RADIUS);
    SYS_Report("N Blocks: %llu\n", currentWorld.validBlocks_);
    //SYS_Report("Start X Z: %zd %zd\n", startX, startZ);
    
    MemoryUsedByVoxel = Memory::getTotalMemoryUsed() - MemoryUsedByVoxel;

    Tick currentTick;
    FVec3 lightPos{0, 25, 0};
    
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
        Renderer::Set3DMode(currentCam); // Configura el modo 3D //Projection
        Renderer::PrepareToRenderInVX0(true, true, true, true);
        Renderer::ObjectView(lightPos.x, lightPos.y, lightPos.z);
        //Renderer::RenderSphere(1, 20, 20, true, 0xFFFF00FF);

        u16 tileTexCoords[4][2] = {
            {0, 0},
            {1, 0},
            {1, 1},
            {0, 1}
        };

        if(options.lightning) {
            Renderer::SetLightDiffuse(0, lightPos, 20, 1);
        }
        
        Renderer::BindTexture(blocksTexture, 0);
        Renderer::SetTextureCoordScaling(0, TILE_SIZE, TILE_SIZE);
        //Renderer::DisableBlend();
        
        nDrawCalls = 0;
        currentTick.start();
        Renderer::PrepareToRenderInVX2(true, true, true, true);
        auto& chunkitos = currentWorld.getChunks();
        for(auto& chunkito : chunkitos) {
            chunkito->render();
        }
        
        Renderer::ObjectView(lightPos.x, lightPos.y, lightPos.z);
        GX_Begin(GX_QUADS, GX_VTXFMT2, 8);
        for (int i = 0; i < 2; ++i) {
            // Dibujar un quad (triángulo) en la cara superior
            for (int j = 0; j < 4; j++) {
                GX_Position3u16(grassQuadFaces[i][j][0],
                                grassQuadFaces[i][j][1],
                                grassQuadFaces[i][j][2]);
                GX_Normal3s8(grassNormals[i][0],
                             grassNormals[i][1],
                             grassNormals[i][2]);
                GX_Color4u8(255, 255, 255, 255);  // Color blanco


                GX_TexCoord2u16(tileTexCoords[j][0] + 1, tileTexCoords[j][1] + 4);
            }
        }
        GX_End();
        
        //currentWorld.renderChunksAround(currentCam.getPosition().x, currentCam.getPosition().z);

        if(options.lightning) Renderer::SetLightOff();
        
        if(options.boundingBox) {
            //-----GRRLIB_SetLightOff();
            Renderer::PrepareToRenderInVX2(true, false, true, false);
            for(const auto& chunkito : chunkitos) {
                Renderer::RenderBoundingBox(chunkito->worldPosition_.x, 0, chunkito->worldPosition_.z, CHUNK_SIZE, UCVec3{0, 255, 255}, true);
            } 
        }
        
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        auto drawTicks = currentTick.stopAndGetTick();
        currentTick.reset();

        
        auto& camPos = currentCam.getPosition();
        // Switch to 2D Mode to display text
        if(options.debugUI) {
            Renderer::Set2DMode();
            text.beginRender();
            //Timings
            text.render(USVec2{5,   5}, fmt::format("Ticks (CPU) : {}", gettick()).c_str());
            text.render(USVec2{5,  20}, fmt::format("Time        : {}", gettime()).c_str());
            text.render(USVec2{5,  35}, fmt::format("Current Time: {}", Engine::getCurrentTime()).c_str());
            text.render(USVec2{5,  50}, fmt::format("Last Time   : {}", Engine::getLastTime()).c_str());
            text.render(USVec2{5,  65}, fmt::format("Delta Time  : {:.3f} s", deltaTime).c_str());
            
            //Memory Things
            text.render(USVec2{5,  110}, fmt::format("Memory (System)   : {:.2f} KB", convertBytesToKilobytes(MemoryUsedBySystem)).c_str());
            text.render(USVec2{5,  125}, fmt::format("Memory (Voxel)    : {:.2f} KB", convertBytesToKilobytes(MemoryUsedByVoxel)).c_str());
            text.render(USVec2{5,  140}, fmt::format("Total Used Memory : {:.2f} KB", convertBytesToKilobytes(Memory::getTotalMemoryUsed())).c_str());
            text.render(USVec2{5,  155}, fmt::format("Total Free Memory : {:.2f} KB", convertBytesToKilobytes(Memory::getTotalMemoryFree())).c_str());

            //Camera Things
            text.render(USVec2{275,  5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
            text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());

            //Render Things
            text.render(USVec2{275,  50}, fmt::format("Valid Blocks : {}", currentWorld.validBlocks_).c_str());
            text.render(USVec2{275,  65}, fmt::format("NDraw Calls  : {}", Renderer::DrawCalls()).c_str());
            text.render(USVec2{275,  80}, fmt::format("NFaces Drawn : {}", Renderer::FacesDrawn()).c_str());

            text.render(USVec2{450, 50}, fmt::format("Video Mode : {}", std::array{"INTERLACE", "NON INTERLACE", "PROGRESSIVE"}[static_cast<int>(Renderer::VideoMode())]).c_str());
            text.render(USVec2{450, 65}, fmt::format("VSYNC      : {}", options.VSYNC ? "YES" : "NOP").c_str());
            text.render(USVec2{450, 80}, fmt::format("NTrees     : {}", currentWorld.nTrees_).c_str());
            

            // text.render(USVec2{400,  65}, fmt::format("Draw Cycles : {} ts", formatThousands(drawTicks)).c_str());
            // text.render(USVec2{400,  80}, fmt::format("Draw Time   : {} ms", Tick::TickToMs(drawTicks)).c_str());
            //text.render(USVec2{400,  95}, fmt::format("Helper      : {}", currentWorld.helperCounter).c_str());
            //text.render(USVec2{400, 110}, fmt::format("N Blocks    : {}", currentChunk.validBlocks).c_str());


        }
        
        //GRRLIB_PrintfTTF(50, 50, myFont, "MINECRAFT", 16, 0x000000FF);

        // Renderizar todo a la pantalla
        Renderer::RenderGX(options.VSYNC); // Render the frame buffer to the TV
    }

    Renderer::Exit();

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

