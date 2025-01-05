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
#include <fstream>
#include <main.h>

using namespace poyo;

#define TILE_SIZE 16

struct Options {
    bool boundingBox = false;
#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
    bool lightning = false;
#endif
    bool debugUI = true;
    bool VSYNC = true;
    bool helper = false;
    bool chunksAround = false;
    int ChunkLoadRadius = CHUNK_LOAD_RADIUS;
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
//3D example from GGRLIB, deform a textured cube
//GRRLIB Basic Drawing has good fonts
//GRRLIB Bitmaps tiene efectos de framebuffer guapos como pixelate y blur
//GRRLIB Compositiing mola tambien
//GRRLIB TTF demo its cool too
//tutorial de particulas 2D en GRRLIB
//GRRLIB Enviromental Mapping, tah guapo
//Fix text Renderer memory

//Lesson 19, Particles!!

u8 waterTexCoords[8];

void updateWaterTextureCoordinates(int textureCounter, u8 offset = 5) {
    // Calculamos el desplazamiento base de las coordenadas en función de textureCounter
    u8 baseOffset = static_cast<u8>(textureCounter / 4);  // Los niveles cambian en 4, es decir, 0, 4, 8, 12, ...

    waterTexCoords[0] = 0 + baseOffset;     waterTexCoords[1] = 0 + offset;
    waterTexCoords[2] = 1 + baseOffset;     waterTexCoords[3] = 0 + offset;
    waterTexCoords[4] = 1 + baseOffset;     waterTexCoords[5] = 1 + offset;
    waterTexCoords[6] = 0 + baseOffset;     waterTexCoords[7] = 1 + offset;
}

static u8 CalculateFrameRate(void) {
    static u8 frameCount = 0;
    static u32 lastTime;
    static u8 FPS = 0;
    const u32 currentTime = Tick::TickToMs(gettime());

    frameCount++;
    if(currentTime - lastTime > 1000) {
        lastTime = currentTime;
        FPS = frameCount;
        frameCount = 0;
    }
    return FPS;
}

#ifdef KIRBY_EASTER_EGG
#include <kirbyinfo.h>
#endif

#include <fat.h>
//1 ms = 40,500 ticks
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
    //GX_SetZCompLoc(GX_FALSE);
    
    Renderer::SetAlphaTest(true);
    
    World currentWorld;

    SYS_Report("N Blocks: %llu\n", 0);
    SYS_Report("N Blocks: %s\n", "He entrado");
    currentWorld.generateLand(CHUNK_RADIUS);
    SYS_Report("N Blocks: %s\n", "He salido");
    //SYS_Report("N Blocks: %llu\n", currentWorld.validBlocks_);
    //SYS_Report("Start X Z: %zd %zd\n", startX, startZ);
    
    MemoryUsedByVoxel = Memory::getTotalMemoryUsed() - MemoryUsedByVoxel;

    Tick currentTick;
    Tick frameTick;
    U64 frameTicks = 0;
    FVec3 lightPos{0, 25, 0};


    // auto memoryUsedByStruct = Memory::getTotalMemoryUsed();
    // auto cubeFacePointer = (CubeFace*)calloc(32, sizeof(CubeFace));
    // cubeFacePointer->tile = 4;
    // memoryUsedByStruct = Memory::getTotalMemoryUsed() - memoryUsedByStruct;
    
    //Start from the first GX command after VSync, and end after GX_DrawDone().
    // GX_SetDrawDone();
    // Loop forever
    int textureCounter = 0;
    float textureCounterFloat = 0.0f;
    float TextureTime = 0.33f;

    std::string helpValue;
    if(fatInitDefault()) {
        helpValue = "Working";
    }else {
        helpValue = "Not working :/";
    }

    FILE *file = fopen("hola2.txt", "w");
    if (file == nullptr) {
        helpValue = "Error opening file";
    }

    if(file != nullptr) {
        fprintf(file, "Hello from GameCube!\n");
        fclose(file);
    }
    
#ifdef KIRBY_EASTER_EGG
    kirbyInfo info;
#endif
    
    while(1) {
        frameTick.start();
        Engine::UpdateEngine();
        auto deltaTime = Engine::getDeltaTime();
        Renderer::ResetDrawCalls();
#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(options.lightning) {
            angle+=deltaTime;
            updatePosition(lightPos, 20, angle);
        }
#endif

        textureCounterFloat += deltaTime;
    
        // Cada vez que textureCounterFloat alcanza o supera 2
        if (textureCounterFloat >= TextureTime) {
            // Incrementa textureCounter en 4
            textureCounter += 4;
        
            // Resetea textureCounterFloat para comenzar un nuevo ciclo
            textureCounterFloat = 0.0f;
        
            // Mantiene textureCounter en el rango 0, 4, 8
            if (textureCounter > 28) {
                textureCounter = 0;
            }
        }

        
        
        Renderer::Set2DMode();
        PAD_ScanPads(); // Scan the GameCube controllers

        // If [START/PAUSE] was pressed on the first GameCube controller, break out of the loop
        if(PAD_ButtonsDown(0) & PAD_BUTTON_START) break;
        if(PAD_ButtonsDown(0) & PAD_TRIGGER_Z) currentCam.setPosition(FVec3(0));
        if(PAD_ButtonsDown(0) & PAD_BUTTON_UP) options.debugUI = !options.debugUI;;
        if(PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT) options.boundingBox = !options.boundingBox;;
#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(PAD_ButtonsDown(0) & PAD_BUTTON_LEFT) options.lightning = !options.lightning;
#endif
        if(PAD_ButtonsDown(0) & PAD_BUTTON_DOWN) options.VSYNC = !options.VSYNC;
        if(PAD_ButtonsDown(0) & PAD_BUTTON_B) {
            CHUNK_LOAD_RADIUS++;
            if(CHUNK_LOAD_RADIUS > CHUNK_RADIUS) {
                CHUNK_LOAD_RADIUS = 1;
            }
        }
        if(PAD_ButtonsDown(0) & PAD_BUTTON_A) options.chunksAround = !options.chunksAround;
        if(PAD_ButtonsDown(0) & PAD_TRIGGER_R) currentCam.setSpeed(CameraSpeed * 5.0f);
        if(PAD_ButtonsUp(0) & PAD_TRIGGER_R) currentCam.setSpeed(CameraSpeed);

#ifdef KIRBY_EASTER_EGG
        updateKirbyPosition(info, deltaTime, 15.0, 1);
#endif
        
        currentCam.updateCamera(deltaTime); //deltaTime
        //-----
        Renderer::Set3DMode(currentCam); // Configura el modo 3D //Projection

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(options.lightning) {
            Renderer::PrepareToRenderInVX0(true, true, true, false);
            Renderer::ObjectView(lightPos.x, lightPos.y, lightPos.z);
            Renderer::RenderSphere(1, 20, 20, true, 0xFFFF00FF);
        }
#endif

        Renderer::BindTexture(blocksTexture, 0);
        Renderer::SetTextureCoordScaling(0, TILE_SIZE, TILE_SIZE);

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA      
        if(options.lightning) {
            Renderer::SetLightDiffuse(0, lightPos, 20, 1);
        }
#endif
        
#ifdef KIRBY_EASTER_EGG
        static int counter = 0;
        counter ++;
        //Renderer::ObjectView(0, 15, 0, 0, counter % 360, 0, 0.05, 0.05, 0.05);
        cfloat kirbyScale = 0.0075f;
        Renderer::ObjectView(info.Position.x, info.Position.y, info.Position.z,
                                info.Rotation.x, info.Rotation.y, info.Rotation.z,
                                kirbyScale, kirbyScale, kirbyScale);
        //Renderer::ObjectView(0, 15, 0, 0, -90, 0, 0.05, 0.05, 0.05);
        //drawKirby();
        drawKirbyFINAL();
#endif
        
        currentTick.start();
        updateWaterTextureCoordinates(textureCounter, 6);

#ifdef OPTIMIZATION_VERTEX_MEMORY
    #ifndef OPTIMIZATION_NO_LIGHTNING_DATA
            Renderer::PrepareToRenderInVX2(true, true, false, true);
    #else
            Renderer::PrepareToRenderInVX2(true, false, false, true);
    #endif
        GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
#else
    #ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        Renderer::PrepareToRenderInVX2(true, false, true, true);
    #else
        Renderer::PrepareToRenderInVX2(true, true, true, true);
    #endif
#endif
        
        Renderer::BindTexture(blocksTexture, 0);
        //Renderer::SetTextureCoordScaling(0, TILE_SIZE, TILE_SIZE);
        auto& chunkitos = currentWorld.getChunks();
        // for(auto& chunkito : chunkitos) {
        //     chunkito->render();
        // }
        //
        // GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);
        // GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
        //
        // GX_SetArray(GX_VA_TEX0, waterTexCoords, 2 * sizeof(u8));
        // DCStoreRange(waterTexCoords, 8 * sizeof(u8));
        // GX_InvVtxCache();
        //
        // for(auto& chunkito : chunkitos) {
        //     chunkito->renderTranslucents();
        // }

        //currentWorld.render(waterTexCoords);


        // Renderer::PrepareToRenderInVX2(true, false, true, true);
        // updateWaterTextureCoordinates(textureCounter, 6);
        //
        // GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX8);
        // GX_SetArray(GX_VA_TEX0, waterTexCoords, 2 * sizeof(u8));
        // GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
        if(options.chunksAround) {
            currentWorld.renderChunksAround(currentCam.getPosition().x, currentCam.getPosition().z, waterTexCoords);
            //currentWorld.renderChunksAround(-18, -8);
        }else {
            currentWorld.render(waterTexCoords);
        }
        
        
        
        // Renderer::ObjectView(lightPos.x, lightPos.y, lightPos.z);
        // GX_Begin(GX_QUADS, GX_VTXFMT2, 8);
        // for (int i = 0; i < 2; ++i) {
        //     // Dibujar un quad (triángulo) en la cara superior
        //     for (int j = 0; j < 4; j++) {
        //         GX_Position3u16(grassQuadFaces[i][j][0],
        //                         grassQuadFaces[i][j][1],
        //                         grassQuadFaces[i][j][2]);
        //         GX_Normal3s8(grassNormals[i][0],
        //                      grassNormals[i][1],
        //                      grassNormals[i][2]);
        //         GX_Color4u8(255, 255, 255, 255);  // Color blanco
        //
        //
        //         GX_TexCoord2u16(tileTexCoords[j][0] + 1, tileTexCoords[j][1] + 4);
        //     }
        // }
        // GX_End();

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(options.lightning) Renderer::SetLightOff();
#endif
        
        if(options.boundingBox) {
            Renderer::PrepareToRenderInVX2(true, false, true, false);
            for(const auto& chunkito : chunkitos) {
                Renderer::RenderBoundingBox(chunkito->worldPosition_.x, 0, chunkito->worldPosition_.z, CHUNK_SIZE, CHUNK_HEIGHT, UCVec3{0, 255, 255}, true);
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
            text.render(USVec2{5,  65}, fmt::format("Delta Time  : {:.5f} s", deltaTime).c_str());
            
            //Memory Things
            text.render(USVec2{5,  110}, fmt::format("Memory (System)   : {:.2f} KB", convertBytesToKilobytes(MemoryUsedBySystem)).c_str());
            text.render(USVec2{5,  125}, fmt::format("Memory (Voxel)    : {:.2f} KB", convertBytesToKilobytes(MemoryUsedByVoxel)).c_str());
            text.render(USVec2{5,  140}, fmt::format("Total Used Memory : {:.2f} KB", convertBytesToKilobytes(Memory::getTotalMemoryUsed())).c_str());
            text.render(USVec2{5,  155}, fmt::format("Total Free Memory : {:.2f} KB", convertBytesToKilobytes(Memory::getTotalMemoryFree())).c_str());

            //Camera Things
            text.render(USVec2{275,  5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
            text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());

            //Render Things
            text.render(USVec2{275,  35}, fmt::format("Valid Blocks : {}", currentWorld.validBlocks_).c_str());
            text.render(USVec2{275,  50}, fmt::format("Valid Faces : {}", currentWorld.validFaces_).c_str());
            text.render(USVec2{275,  65}, fmt::format("NDraw Calls  : {}", Renderer::DrawCalls()).c_str());
            //text.render(USVec2{275,  80}, fmt::format("NFaces Drawn : {}", Renderer::FacesDrawn()).c_str());  //todo: fix
            text.render(USVec2{275,  80}, fmt::format("NChunks      : {}", currentWorld.NChunks()).c_str());

            text.render(USVec2{450, 50}, fmt::format("Video Mode : {}", std::array{"INTERLACE", "NON INTERLACE", "PROGRESSIVE"}[static_cast<int>(Renderer::VideoMode())]).c_str());
            text.render(USVec2{450, 65}, fmt::format("VSYNC      : {}", options.VSYNC ? "YES" : "NOP").c_str());
            text.render(USVec2{450, 80}, fmt::format("NTrees     : {}", currentWorld.nTrees_).c_str()); //currentWorld.nTrees_
            
            text.render(USVec2{400,   95}, fmt::format("Draw Cycles  : {} ts", drawTicks).c_str());
            text.render(USVec2{400,   110}, fmt::format("Frame Cycles : {} ts", frameTicks).c_str());
            text.render(USVec2{400,  125}, fmt::format("Draw Time    : {} ms", Tick::TickToMsfloat(drawTicks)).c_str());
            text.render(USVec2{400,  140}, fmt::format("Helper      : {}", helpValue).c_str());
            //text.render(USVec2{400,  95}, fmt::format("Helper      : {}", currentWorld.helperCounter).c_str());
            //text.render(USVec2{400, 110}, fmt::format("N Blocks    : {}", currentChunk.validBlocks).c_str());


        }
        
        //GRRLIB_PrintfTTF(50, 50, myFont, "MINECRAFT", 16, 0x000000FF);

        frameTicks = frameTick.stopAndGetTick();
        frameTick.reset();
        
        // Renderizar todo a la pantalla
        Renderer::RenderGX(options.VSYNC); // Render the frame buffer to the TV
    }

    Renderer::Exit();

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

