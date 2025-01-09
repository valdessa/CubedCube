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

#include <fstream>

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

float CameraSpeed = 15.0f;

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
float angle = 0.0f;
FVec3 lightPos{0, 25, 0};

void updateLightPosition(FVec3& point, float radius, float angle) {
    // Actualiza las coordenadas X y Z para que el punto se mueva en un círculo
    point.x = radius * cos(angle); // Movimiento circular en el eje X
    point.z = radius * sin(angle); // Movimiento circular en el eje Z
}
#endif

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

#ifdef KIRBY_EASTER_EGG
#include <kirbyinfo.h>
#endif

//TPL and Fonts and Inputs
#include <main.h>
#include <fat.h>

	constexpr float cubePositions[] = {
	     // back face
		-1.0f, -1.0f, -1.0f,  // bottom-left
		 1.0f,  1.0f, -1.0f,   // top-right
		 1.0f, -1.0f, -1.0f,   // bottom-right         
		 1.0f,  1.0f, -1.0f,   // top-right
		-1.0f, -1.0f, -1.0f,   // bottom-left
		-1.0f,  1.0f, -1.0f,   // top-left
		// front face
		-1.0f, -1.0f,  1.0f,   // bottom-left
		 1.0f, -1.0f,  1.0f,   // bottom-right
		 1.0f,  1.0f,  1.0f,   // top-right
		 1.0f,  1.0f,  1.0f,   // top-right
		-1.0f,  1.0f,  1.0f,   // top-left
		-1.0f, -1.0f,  1.0f,   // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f,  // top-right
		-1.0f,  1.0f, -1.0f,  // top-left
		-1.0f, -1.0f, -1.0f,  // bottom-left
		-1.0f, -1.0f, -1.0f,  // bottom-left
		-1.0f, -1.0f,  1.0f,  // bottom-right
		-1.0f,  1.0f,  1.0f,  // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  // top-left
		 1.0f, -1.0f, -1.0f,   // bottom-right
		 1.0f,  1.0f, -1.0f,   // top-right         
		 1.0f, -1.0f, -1.0f,   // bottom-right
		 1.0f,  1.0f,  1.0f,   // top-left
		 1.0f, -1.0f,  1.0f,   // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,   // top-right
		 1.0f, -1.0f, -1.0f,   // top-left
		 1.0f, -1.0f,  1.0f,  // bottom-left
		 1.0f, -1.0f,  1.0f,   // bottom-left
		-1.0f, -1.0f,  1.0f,  // bottom-right
		-1.0f, -1.0f, -1.0f,   // top-right
		// top face
		-1.0f,  1.0f, -1.0f,   // top-left
		 1.0f,  1.0f , 1.0f,   // bottom-right
		 1.0f,  1.0f, -1.0f,   // top-right     
		 1.0f,  1.0f,  1.0f,   // bottom-right
		-1.0f,  1.0f, -1.0f,   // top-left
		-1.0f,  1.0f,  1.0f,   // bottom-left  
	};

	constexpr float cubeTextureCoords[] = {
		// back face
		0.0f, 0.0f, // bottom-left
		1.0f, 1.0f, // top-right
		1.0f, 0.0f, // bottom-right         
		1.0f, 1.0f, // top-right
		0.0f, 0.0f, // bottom-left
		0.0f, 1.0f, // top-left
		// front face
		0.0f, 0.0f, // bottom-left
		1.0f, 0.0f, // bottom-right
		1.0f, 1.0f, // top-right
		1.0f, 1.0f, // top-right
		0.0f, 1.0f, // top-left
		0.0f, 0.0f, // bottom-left
		// left face
		1.0f, 0.0f, // top-right
		1.0f, 1.0f, // top-left
		0.0f, 1.0f, // bottom-left
		0.0f, 1.0f, // bottom-left
		0.0f, 0.0f, // bottom-right
		1.0f, 0.0f, // top-right
		// right face
		1.0f, 0.0f, // top-left
		0.0f, 1.0f, // bottom-right
		1.0f, 1.0f, // top-right         
		0.0f, 1.0f, // bottom-right
		1.0f, 0.0f, // top-left
		0.0f, 0.0f, // bottom-left     
		// bottom face
		0.0f, 1.0f, // top-right
		1.0f, 1.0f, // top-left
		1.0f, 0.0f, // bottom-left
		1.0f, 0.0f, // bottom-left
		0.0f, 0.0f, // bottom-right
		0.0f, 1.0f, // top-right
		// top face
		0.0f, 1.0f, // top-left
		1.0f, 0.0f, // bottom-right
		1.0f, 1.0f, // top-right     
		1.0f, 0.0f, // bottom-right
		0.0f, 1.0f, // top-left
		0.0f, 0.0f  // bottom-left  
	};

	unsigned int cubeIndices[] = {
		// back face
		0, 1, 2,
		3, 4, 5,
		// front face
		6, 7, 8,
		9, 10, 11,
		// left face
		12, 13, 14,
		15, 16, 17,
		// right face
		18, 19, 20,
		21, 22, 23,
		// bottom face
		24, 25, 26,
		27, 28, 29,
		// top face
		30, 31, 32,
		33, 34, 35
	};


void drawTriangleCube() {
	Renderer::PrepareToRenderInVX0(true, false, true, false);
	constexpr auto sizeToUse = sizeof(cubeIndices) / sizeof(unsigned int);
	GX_Begin(GX_TRIANGLES, GX_VTXFMT0, sizeToUse);
	for(int i = 0; i < sizeToUse; i++) {
		auto& index = cubeIndices[i];
		GX_Position3f32(
			cubePositions[index * 3 + 0], // x
			cubePositions[index * 3 + 1], // y
			cubePositions[index * 3 + 2]  // z)
		);
		
		// GX_TexCoord2u8(
		// 	cubeTextureCoords[index * 2 + 0] + 1, // u
		// 	cubeTextureCoords[index * 2 + 1]  // v
		// );
		GX_Color4u8(0, 255, 255, 255);
	}
	GX_End();
}


void drawTriangleStripCube() {
    static constexpr float cube_strip[] = {
        -1.f, 1.f, 1.f,     // Front-top-left
        1.f, 1.f, 1.f,      // Front-top-right
        -1.f, -1.f, 1.f,    // Front-bottom-left
        1.f, -1.f, 1.f,     // Front-bottom-right
        1.f, -1.f, -1.f,    // Back-bottom-right
        1.f, 1.f, 1.f,      // Front-top-right
        1.f, 1.f, -1.f,     // Back-top-right
        -1.f, 1.f, 1.f,     // Front-top-left
        -1.f, 1.f, -1.f,    // Back-top-left
        -1.f, -1.f, 1.f,    // Front-bottom-left
        -1.f, -1.f, -1.f,   // Back-bottom-left
        1.f, -1.f, -1.f,    // Back-bottom-right
        -1.f, 1.f, -1.f,    // Back-top-left
        1.f, 1.f, -1.f      // Back-top-right
    };

    static constexpr uint8_t cube_strip_uvs[] = {
        // UVs for Front-top-left
        0, 1,
        // UVs for Front-top-right
        1, 1,
        // UVs for Front-bottom-left
        0, 0,
        // UVs for Front-bottom-right
        1, 0,
        // UVs for Back-bottom-right
        1, 0,
        // UVs for Front-top-right
        1, 1,
        // UVs for Back-top-right
        1, 1,
        // UVs for Front-top-left
        0, 1,
        // UVs for Back-top-left
        0, 1,
        // UVs for Front-bottom-left
        0, 0,
        // UVs for Back-bottom-left
        0, 0,
        // UVs for Back-bottom-right
        1, 0,
        // UVs for Back-top-left
        0, 1,
        // UVs for Back-top-right
        1, 1
    };


    Renderer::PrepareToRenderInVX0(true, false, true, false);
    GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 14);
    for(int i = 0, j = 0; i < 42; i+=3, j+=2) {
        GX_Position3f32(cube_strip[i], cube_strip[i + 1], cube_strip[i + 2]);
        //GX_Color4u8(glm::abs<u8>(cube_strip[i]), glm::abs<u8>(cube_strip[i + 1]), glm::abs<u8>(cube_strip[i + 2]), 255);
        //GX_TexCoord2u8(cube_strip_uvs[j], cube_strip_uvs[j + 1]);
    	GX_Color4u8(0, 255, 255, 255);
    }

    GX_End();
}

void RenderCube(const Cubito& cube) {
	U8 positionsCoords[6][3] = {
		{1, 0, 0},
		{0, 0, 0},
		{0, 1, 0},
		{0, 0, 0},
		{0, 0, 1},
		{0, 0, 0}
	};
	
	Renderer::ObjectView(cube.x, cube.y, cube.z,
					  0, 0, 0,
					  1.0f, 1.0f, 1.0f);
	
	Renderer::RenderBegin(24);

	for(int i = 0; i < 6; i++) {
		auto& face = cube.face[i]; 
		for (int j = 0; j < 4; j++) {
			GX_Position3u16(positionsCoords[face.direction][0] + cubeFaces[face.direction][j][0],
						positionsCoords[face.direction][1] + cubeFaces[face.direction][j][1],
						positionsCoords[face.direction][2] + cubeFaces[face.direction][j][2]);
			GX_Color4u8(0, 255, 255, 255);
		}
	}
	Renderer::RenderEnd();
}


//1 ms = 40,500 ticks
int main(int argc, char **argv) {
    // size_t MemoryUsedAtbeginning = (uintptr_t)SYS_GetArena1Lo() - (uintptr_t)SYS_GetArena1Hi;
    // MemoryUsedAtbeginning += SYS_GetArena1Size();
    Options options;
    //srand(time(nullptr));

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
    Camera currentCam(FVec3{0.0f, 30.0f, 50.0f}, -20, -90, CameraSpeed);


    auto MemoryUsedByVoxel = Memory::getTotalMemoryUsed();
    
    Renderer::SetAlphaTest(true);
    
    World currentWorld;

    // SYS_Report("N Blocks: %llu\n", 0);
    // SYS_Report("N Blocks: %s\n", "He entrado");
    currentWorld.generateLand(CHUNK_RADIUS);
    //SYS_Report("N Blocks: %s\n", "He salido");
    //SYS_Report("N Blocks: %llu\n", currentWorld.validBlocks_);
    //SYS_Report("Start X Z: %zd %zd\n", startX, startZ);
    
    MemoryUsedByVoxel = Memory::getTotalMemoryUsed() - MemoryUsedByVoxel;

    Tick currentTick;
    Tick frameTick;
    U64 frameTicks = 0;
    U16 chunksDrawn = 0;
    
    //std::string helpValue = SYS_GetAbsolutePath(path);
    std::string helpValue = std::to_string(Renderer::isAntialiased());
    // if(fatInitDefault()) {
    //     helpValue = "Working";
    // }else {
    //     helpValue = "Not working :/";
    // }
    //
    // FILE *file = fopen("hola2.txt", "w");
    // if (file == nullptr) {
    //     helpValue = "Error opening file";
    // }
    //
    // if(file != nullptr) {
    //     fprintf(file, "Hello from GameCube!\n");
    //     fclose(file);
    // }

#ifdef KIRBY_EASTER_EGG
    kirbyInfo info;
    initKirby();
#endif
    
    while(true) {
        frameTick.start();
        Engine::UpdateEngine();
        auto deltaTime = Engine::getDeltaTime();
        Renderer::ResetDrawCalls();
#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(options.lightning) {
            angle+=deltaTime;
            updateLightPosition(lightPos, 20, angle);
        }
#endif
        
        Renderer::Set2DMode();
        if(updateInput(options, currentCam)) break;

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
        //GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        for(int j = -5; j < 5; j++) {
            for(int i = -5; i < 5; i++) {
                Renderer::ObjectView(i, 25 + j, -j + 5, 0, 0, 0, 0.5f, 0.5f, 0.5f);
                drawTriangleStripCube();
            }
        }

    	for(int j = -5; j < 5; j++) {
    		for(int i = -5; i < 5; i++) {
    			Renderer::ObjectView(i - 15, 25 + j, -j + 5, 0, 0, 0, 0.5f, 0.5f, 0.5f);
    			drawTriangleCube();
    		}
    	}

    	

        Renderer::PrepareToRenderInVX2(true, false, true, false);
        for(int j = -5; j < 5; j++) {
            for(int i = -5; i < 5; i++) {
                Cubito newCubito;
                setCubitoStatic(newCubito, CubePosition(i + 15, 25 + j, -j + 5), BLOCK_DIRT);
                RenderCube(newCubito);
            }
        }


        
        auto& kirbys = currentWorld.kirbyTransforms_;
        updateKirbyAnimation();
        for(size_t i = 0; i < kirbys.size(); i++ ) {
            Mtx matrixToUse;
            guMtxIdentity(matrixToUse);
            auto& modelMatrix = kirbys[i].modelMatrix;
            guMtxConcat(Renderer::ViewMatrix(), modelMatrix, matrixToUse);
            drawKirby(matrixToUse, i);
        }
        
#ifdef KIRBY_IN_DISPLAY_LIST
        GX_SetCurrentMtx(GX_PNMTX0); //Reset to original Matrix :3
#endif
        
        // Renderer::ObjectView(info.Position.x + 0.5f, info.Position.y, info.Position.z + 0.5f,
        //                         info.Rotation.x, info.Rotation.y, info.Rotation.z,
        //                         kirbyScale, kirbyScale, kirbyScale);
        // drawKirby();
#endif
        
        currentTick.start();
        updateWaterTextureCoordinates(deltaTime, 6);

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
        
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);

    	if(options.chunksAround) {
            chunksDrawn = currentWorld.renderChunksAround(currentCam.getPosition().x, currentCam.getPosition().z, waterTexCoords);
            //currentWorld.renderChunksAround(-18, -8);
        }else {
            chunksDrawn = currentWorld.render(waterTexCoords);
        }
        
        
#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(options.lightning) Renderer::SetLightOff();
#endif
        
        if(options.boundingBox) {
            GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            Renderer::PrepareToRenderInVX2(true, false, true, false);
            auto& chunkitos = currentWorld.getChunks();
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
            text.render(USVec2{5,  95}, fmt::format("Memory (System)   : {:.2f} KB", convertBytesToKilobytes(MemoryUsedBySystem)).c_str());
            text.render(USVec2{5,  110}, fmt::format("Memory (Voxel)    : {:.2f} KB", convertBytesToKilobytes(MemoryUsedByVoxel)).c_str());
            text.render(USVec2{5,  125}, fmt::format("Total Used Memory : {:.2f} KB", convertBytesToKilobytes(Memory::getTotalMemoryUsed())).c_str());
            text.render(USVec2{5,  140}, fmt::format("Total Free Memory : {:.2f} KB", convertBytesToKilobytes(Memory::getTotalMemoryFree())).c_str());

            //Screen Thinks
            text.render(USVec2{5, 170}, fmt::format("Video Mode : {}", std::array{"INTERLACE", "NON INTERLACE", "PROGRESSIVE", "ERROR"}[static_cast<int>(Renderer::VideoMode())]).c_str());
            text.render(USVec2{5, 185}, fmt::format("VSYNC      : {}", options.VSYNC ? "YES" : "NOP").c_str());
        	text.render(USVec2{5, 200}, fmt::format("Resolution  : {}/{}", Renderer::ScreenWidth(), Renderer::ScreenHeight()).c_str());

            
            //Camera Things
            text.render(USVec2{275,  5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
            text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());

            //Render Things
            text.render(USVec2{275,  35}, fmt::format("Valid: Blocks [{}] Faces [{}]", currentWorld.validBlocks_, currentWorld.validFaces_).c_str());
            text.render(USVec2{275,  50}, fmt::format("NDraw Calls : {}", Renderer::DrawCalls()).c_str());
            //text.render(USVec2{275,  80}, fmt::format("NFaces Drawn : {}", Renderer::FacesDrawn()).c_str());  //todo: fix
            text.render(USVec2{275,  65}, fmt::format("NChunks     : [{}]/[{}]", chunksDrawn, currentWorld.NChunks()).c_str());
            text.render(USVec2{485, 50}, fmt::format("NTrees : {}", currentWorld.nTrees_).c_str());
#ifdef KIRBY_EASTER_EGG
            text.render(USVec2{485, 65}, fmt::format("NKirby : {}", currentWorld.NKirbys).c_str()); //currentWorld.nTrees_
#endif
            text.render(USVec2{295,  95}, fmt::format("Draw   : Cycles {} ts / Time {} ms", drawTicks, Tick::TickToMsfloat(drawTicks)).c_str());
            text.render(USVec2{295, 110}, fmt::format("Frame  : Cycles {} ts", frameTicks).c_str());
            text.render(USVec2{295, 125}, fmt::format("Helper : {}", helpValue).c_str());
            //text.render(USVec2{400,  95}, fmt::format("Helper      : {}", currentWorld.helperCounter).c_str());
            //text.render(USVec2{400, 110}, fmt::format("N Blocks    : {}", currentChunk.validBlocks).c_str());
        }
        
        //GRRLIB_PrintfTTF(50, 50, myFont, "MINECRAFT", 16, 0x000000FF);

        frameTicks = frameTick.stopAndGetTick();
        frameTick.reset();
        
        // Renderizar todo a la pantalla
        Renderer::RenderGX(options.VSYNC); // Render the frame buffer to the TV
    }

#ifdef KIRBY_IN_DISPLAY_LIST
    freeKirby();
#endif

    Renderer::Exit();

    exit(0); // Use exit() to exit a program, do not use 'return' from main()
}

