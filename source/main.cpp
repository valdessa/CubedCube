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
#include <bounding_region.h>
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

enum class RenderChunkMode {
    DEFAULT,
    CHUNKS_AROUND,
#ifdef OPTIMIZATION_FRUSTUM_CULLING
    CHUNKS_IN_FRUSTUM,
#endif
    COUNT
};

struct Options {
    bool boundingBox = false;
#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
    bool lightning = false;
#endif
    bool debugUI = true;
    bool VSYNC = true;
    bool helper = false;
    RenderChunkMode chunkDrawMode = RenderChunkMode::DEFAULT;
    int ChunkLoadRadius = CHUNK_LOAD_RADIUS;
#ifdef ENABLE_AUTOMATIC_CAMERA
    bool isInterpolating = false; // Flag to track if interpolation is running
    float t = 0.0f; // Interpolation factor
#endif
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

#ifdef OPTIMIZATION_FRUSTUM_CULLING
static Frustum frustum;

float far_ = 75.0f;

void updateFrustum(Camera& cam) {
    far_ = CHUNK_LOAD_RADIUS * CHUNK_SIZE;
    const float halfVSide = far_ * tanf(glm::radians(cam.fov_) * .5f);
    const float halfHSide = halfVSide * cam.aspectRatio_;
    const glm::vec3 frontMultFar = far_ * cam.forward_;

    frustum.nearFace = { cam.getPosition() + cam.near_ * cam.forward_, cam.forward_ };
    frustum.farFace = { cam.getPosition() + frontMultFar, -cam.forward_ };
    frustum.rightFace = { cam.getPosition(), glm::cross(frontMultFar - cam.right_ * halfHSide, cam.up_) };
    frustum.leftFace = { cam.getPosition(), glm::cross(cam.up_, frontMultFar + cam.right_ * halfHSide) };
    frustum.topFace = { cam.getPosition(), glm::cross(cam.right_, frontMultFar - cam.up_ * halfVSide) };
    frustum.bottomFace = { cam.getPosition(), glm::cross(frontMultFar + cam.up_ * halfVSide, cam.right_) };
}
#endif

#ifdef ENABLE_MEASUREMENTS
//Only Once
struct Measurements {
    size_t MemoryUsedBySystem;
    size_t MemoryUsedByVoxel;
    size_t TotalMemoryUsed;
    size_t TotalMemoryFree;
    RenderChunkMode chunkDrawMode;
    U32 NTotalChunks;
    U32 NValidBlocks;
    U32 NValidFaces;
};
//Once per frame
struct MeasurementsByFrame {
    float frameTimeInMs;
    float DrawTimeInMs;
    U32 NChunksDrawn;
    U32 NDrawCalls;
    u32 NFacesDrawn;
};

#include <algorithm>  // Para std::sort
#include <numeric>    // Para std::accumulate

// Función para calcular la media
template <typename T>
T calculateMean(const std::vector<T>& values) {
    if (values.empty()) return 0;
    T sum = std::accumulate(values.begin(), values.end(), T(0));
    return sum / values.size();
}

// Función para calcular la mediana
template <typename T>
T calculateMedian(std::vector<T> values) {
    if (values.empty()) return 0;
    std::sort(values.begin(), values.end());
    size_t size = values.size();
    if (size % 2 == 0) {
        return (values[size / 2 - 1] + values[size / 2]) / 2;
    } else {
        return values[size / 2];
    }
}

class MeasurementSystem {
    Measurements initialMeasurement;
    Vector<MeasurementsByFrame> frameMeasurements;
    U32 framesToMeasure;
    U32 currentFrame;
    bool shouldStart = false;

public:
    MeasurementSystem(U32 numFrames, String& helpValue)
        : initialMeasurement(), frameMeasurements(numFrames), framesToMeasure(numFrames), currentFrame(0) {
        
        if(fatInitDefault()) {
            helpValue = "Working";
        }else {
            helpValue = "Not working :/";
        }
    }

    U32 getCurrentFrame() const {return currentFrame; }

    void setInitialMeasurement(const Measurements& measure) {
        initialMeasurement = measure;
        shouldStart = true;
        currentFrame = 0;
    }

    bool shouldRecordFrame() const {
        return shouldStart;
    }
    
    // Log data per frame
    void recordFrame(MeasurementsByFrame& frameMeasurement) {
        if (currentFrame < framesToMeasure) {
            frameMeasurements[currentFrame++] = std::move(frameMeasurement);
            //frameMeasurements.push_back(frameMeasurement);
        }
    }

    // Check if the measurement is already complete
    bool isMeasurementComplete() const {
        return currentFrame >= framesToMeasure;
    }

    // Dump data to a .txt file
    void dumpToFile(cString& filename) {
        shouldStart = false;
        std::ofstream file(filename);
        if (!file.is_open()) {
            exit(-2);
            //std::cerr << "Error al abrir el archivo " << filename << "\n";
            return;
        }

        // Escribir mediciones iniciales
        file << "Initial Measurements:\n";
        file << "Memory Used By System: " << convertBytesToKilobytes(initialMeasurement.MemoryUsedBySystem) << " MB\n";
        file << "Memory Used By Voxel: " << convertBytesToKilobytes(initialMeasurement.MemoryUsedByVoxel) << " MB\n";
        file << "Total Memory Used: " << convertBytesToKilobytes(initialMeasurement.TotalMemoryUsed) << " MB\n";
        file << "Total Memory Free: " << convertBytesToKilobytes(initialMeasurement.TotalMemoryFree) << " MB\n";
        String ChunkRenderMode = "DEFAULT";
        switch (initialMeasurement.chunkDrawMode) {
            case RenderChunkMode::CHUNKS_AROUND:        ChunkRenderMode = "AROUND THE CAMERA"; break;
        #ifdef OPTIMIZATION_FRUSTUM_CULLING
            case RenderChunkMode::CHUNKS_IN_FRUSTUM:    ChunkRenderMode = "FRUSTUM CULLED";    break;
        #endif
        }
        file << "Chunk Draw Mode: " << ChunkRenderMode << "\n";
        file << "Total Chunks: " << initialMeasurement.NTotalChunks << "\n";
        file << "Valid Blocks: " << initialMeasurement.NValidBlocks << "\n";
        file << "Valid Faces: " << initialMeasurement.NValidFaces << "\n\n";

        // Escribir mediciones por frame
        file << "Frame Measurements:\n";
        for (size_t i = 0; i < frameMeasurements.size(); ++i) {
            const auto& frame = frameMeasurements[i];
            file << "Frame " << i + 1 << ":\n";
            file << "  Frame Time: " << frame.frameTimeInMs << " ms\n";
            file << "  Draw Time: " << frame.DrawTimeInMs << " ms\n";
            file << "  Chunks Drawn: " << frame.NChunksDrawn << "\n";
            file << "  Draw Calls: " << frame.NDrawCalls << "\n";
            file << "  Faces Drawn: " << frame.NFacesDrawn << "\n";
        }

        file.close();
        //std::cout << "Mediciones volcadas en " << filename << "\n";
    }

    // Dump data to a .csv file
    void dumpToCSV(cString& filename) {
        shouldStart = false;
        std::ofstream file(filename);
        if (!file.is_open()) {
            exit(-2);
            //std::cerr << "Error opening the file " << filename << "\n";
            return;
        }

        file << "FRAME,FRAME TIME (ms),DRAW TIME (ms),CHUNKS DRAWN,DRAW CALLS,FACES DRAWN\n";

        // Variables para estadísticas por frame
        Vector<float> frameTimes;
        Vector<float> drawTimes;
        Vector<U32> chunksDrawn;
        Vector<U32> drawCalls;
        Vector<U32> facesDrawn;

        // Write measurements per frame
        for (size_t i = 0; i < frameMeasurements.size(); ++i) {
            const auto& frame = frameMeasurements[i];

            frameTimes.push_back(frame.frameTimeInMs);
            drawTimes.push_back(frame.DrawTimeInMs);
            chunksDrawn.push_back(frame.NChunksDrawn);
            drawCalls.push_back(frame.NDrawCalls);
            facesDrawn.push_back(frame.NFacesDrawn);
            
            file << "Frame " << i + 1 << ","
                 << frame.frameTimeInMs << ","
                 << frame.DrawTimeInMs << ","
                 << frame.NChunksDrawn << ","
                 << frame.NDrawCalls << ","
                 << frame.NFacesDrawn << "\n";
        }

        file << "\n\n";
        
        file << "*-*-*-*-*,*-*-*-*-*,*-*-*-*-*,STATISTICS OF:, [" << filename << "],*-*-*-*-*,*-*-*-*-*,*-*-*-*-*\n";
        file << ",TYPE, MAX, MIN, MEAN, MEDIAN\n";
        
        // Write statistics at the end of the CSV file
        calculateStatistics(file, "Frame Times", frameTimes);
        calculateStatistics(file, "Draw Times", drawTimes);
        calculateStatistics(file, "Chunks Drawn", chunksDrawn);
        calculateStatistics(file, "Draw Calls", drawCalls);
        calculateStatistics(file, "Faces Drawn", facesDrawn);

        file << "\n";
        
        // Write headers
        file << "MEM USED [SYS](KB),MEM USED [VOXEL](KB),TOTAL MEM USED(KB),TOTAL MEM FREE(KB),CHUNK DRAW MODE,TOTAL CHUNKS,VALID BLOCKS,VALID FACES\n";

        String ChunkRenderMode = "DEFAULT";
        switch (initialMeasurement.chunkDrawMode) {
            case RenderChunkMode::CHUNKS_AROUND:        ChunkRenderMode = "AROUND THE CAMERA"; break;
        #ifdef OPTIMIZATION_FRUSTUM_CULLING
            case RenderChunkMode::CHUNKS_IN_FRUSTUM:    ChunkRenderMode = "FRUSTUM CULLED";    break;
        #endif
        }
        
        // Write initial measurement
        file << convertBytesToKilobytes(initialMeasurement.MemoryUsedBySystem) << ","
             << convertBytesToKilobytes(initialMeasurement.MemoryUsedByVoxel) << ","
             << convertBytesToKilobytes(initialMeasurement.TotalMemoryUsed) << ","
             << convertBytesToKilobytes(initialMeasurement.TotalMemoryFree) << ","
             << ChunkRenderMode << ","
             << initialMeasurement.NTotalChunks << ","
             << initialMeasurement.NValidBlocks << ","
             << initialMeasurement.NValidFaces << "\n";

        file.close();
    }
    
    template <typename T>
    void calculateStatistics(std::ofstream& file, cString& name, Vector<T>& data) {
        T max = *std::max_element(data.begin(), data.end());
        T min = *std::min_element(data.begin(), data.end());
        T mean = calculateMean(data);
        T median = calculateMedian(data);
        
        // Write statistics at the end of the CSV file
        file << ","
             << name << ","
             << max << ","
             << min << ","
             << mean << ","
             << median << "\n";
    }
    
};
#endif

#ifdef ENABLE_AUTOMATIC_CAMERA
void moveCameraToTargetByFrame(Camera& camera, Options& opt, U32 currentFrame, cFVec3& startPos, cFVec3& endPos, float startPitch, float endPitch, float startYaw, float endYaw, U32 MaxFrame) {
    if (opt.isInterpolating) {
        // Compute the interpolation factor t
        float t = static_cast<float>(currentFrame) / static_cast<float>(MaxFrame);
        opt.t = glm::clamp(t, 0.0f, 1.0f); // Ensure t stays in range [0, 1]

        if (opt.t >= 1.0f) {
            opt.t = 1.0f; // Ensure t doesn't go beyond 1
            opt.isInterpolating = false; // Stop interpolation once it reaches the end
        }

        // Interpolate position and rotation
        camera.interpolate(startPos, endPos, startPitch, endPitch, startYaw, endYaw, opt.t);
    }
}
void moveCameraToTarget(Camera& camera, Options& opt, float deltaTime, cFVec3& startPos, cFVec3& endPos, float startPitch, float endPitch, float startYaw, float endYaw, float duration) {
    if (opt.isInterpolating) {
        opt.t += deltaTime / duration;  // deltaTime is the time passed per frame

        if (opt.t >= 1.0f) {
            opt.t = 1.0f; // Ensure t doesn't go beyond 1
            opt.isInterpolating = false; // Stop interpolation once it reaches the end
        }

        // Interpolate position and rotation
        camera.interpolate(startPos, endPos, startPitch, endPitch, startYaw, endYaw, opt.t);
    }
}
#endif


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

#ifdef KIRBY_EASTER_EGG
    kirbyInfo info;
    initKirby();
#endif

#ifdef ENABLE_MEASUREMENTS
    MeasurementSystem measurement_system(MEASUREMENTS_FRAMES, helpValue);
#endif
#ifdef ENABLE_AUTOMATIC_CAMERA
    auto beginPos = currentWorld.getChunks().front()->worldPosition_;
    auto endPos = currentWorld.getChunks().back()->worldPosition_;
    FVec3 CameraInitialPos(beginPos.x - CHUNK_SIZE/2, CHUNK_HEIGHT, beginPos.z - CHUNK_SIZE/2);
    FVec3 CameraFinalPos(endPos.x + CHUNK_SIZE * 1.5, CHUNK_HEIGHT, endPos.z + CHUNK_SIZE * 1.5);
    currentCam.setPosition(CameraInitialPos);
    float cameraInitialYaw = 45;
    float cameraFinalYaw = -135;
    currentCam.yaw_ = cameraInitialYaw;
    //currentCam.yaw_ = 45.0f;


    //final yaw = -135
#endif

#ifdef CHUNK_RENDER_MODE
    options.chunkDrawMode = static_cast<RenderChunkMode>(CHUNK_RENDER_MODE);
#endif
    
    auto fileName = Engine::generateOptimizationsString();
    
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

#ifdef ENABLE_MEASUREMENTS
        if(PAD_ButtonsDown(1) & PAD_BUTTON_START) {
            Measurements m;
            m.MemoryUsedBySystem = MemoryUsedBySystem;
            m.MemoryUsedByVoxel = MemoryUsedByVoxel;
            m.TotalMemoryUsed = Memory::getTotalMemoryUsed();
            m.TotalMemoryFree = Memory::getTotalMemoryFree();
            m.chunkDrawMode = options.chunkDrawMode;
            m.NTotalChunks = currentWorld.NChunks();
            m.NValidBlocks = currentWorld.validBlocks_;
            m.NValidFaces = currentWorld.validFaces_;
            
            measurement_system.setInitialMeasurement(m);
            helpValue = "Recording...";
        }
#endif

#ifdef ENABLE_AUTOMATIC_CAMERA
        if(PAD_ButtonsDown(1) & PAD_BUTTON_START) {
            currentCam.setPosition(CameraInitialPos);
            options.isInterpolating = true;
            options.t = 0.0f; // Reset t to start from the beginning
        }
    #ifdef ENABLE_MEASUREMENTS
        moveCameraToTargetByFrame(currentCam, options, measurement_system.getCurrentFrame(), CameraInitialPos, CameraFinalPos,
            currentCam.pitch_, currentCam.pitch_, cameraInitialYaw, cameraFinalYaw, MEASUREMENTS_FRAMES);
    #else
        moveCameraToTarget(currentCam, options, deltaTime, CameraInitialPos, CameraFinalPos,
            currentCam.pitch_, currentCam.pitch_, cameraInitialYaw, cameraFinalYaw, ENABLE_AUTOMATIC_CAMERA);
    #endif
#endif
        
        currentCam.updateCamera(deltaTime); //deltaTime
#ifdef OPTIMIZATION_FRUSTUM_CULLING
        updateFrustum(currentCam);
#endif
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
        auto& kirbys = currentWorld.kirbyTransforms_;
        updateKirbyAnimation();
        for(size_t i = 0; i < kirbys.size(); i++ ) {
            Mtx matrixToUse;
            guMtxIdentity(matrixToUse);
            auto& modelMatrix = kirbys[i].modelMatrix;
            guMtxConcat(Renderer::ViewMatrix(), modelMatrix, matrixToUse);
            drawKirby(matrixToUse, i);
        }
    #ifdef KIRBY_CONTROLLED
        updateKirbyPosition(info, deltaTime, 15.0, 1);
        Mtx matrixToUse;
        Renderer::CalculateModelMatrix(matrixToUse, info.trans);
        guMtxConcat(Renderer::ViewMatrix(), matrixToUse, matrixToUse);
        drawKirby(matrixToUse, MAX_KIRBY);
    #endif
    #ifdef KIRBY_IN_DISPLAY_LIST
            GX_SetCurrentMtx(GX_PNMTX0); //Reset to original Matrix :3
    #endif
#endif

        //Chunks to Draw:
        Vector<Chunk*> chunksToRender;
        switch (options.chunkDrawMode) {
        case RenderChunkMode::CHUNKS_AROUND:
            currentWorld.calculateChunksAround(currentCam.getPosition().x, currentCam.getPosition().z, chunksToRender);
            break;
#ifdef OPTIMIZATION_FRUSTUM_CULLING
        case RenderChunkMode::CHUNKS_IN_FRUSTUM:   
            currentWorld.calculateChunksInFrustum(frustum, chunksToRender);
            break;
#endif
        }

        //---
        
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
        Renderer::PrepareToRenderInVX2(true, true, true, true);
    #else
        Renderer::PrepareToRenderInVX2(true, false, true, true);
    #endif
#endif
        
        GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
        switch (options.chunkDrawMode) {
            case RenderChunkMode::DEFAULT:
                chunksDrawn = currentWorld.render(waterTexCoords);
                break;
            case RenderChunkMode::CHUNKS_AROUND:
                //OLD METHOD:
                //chunksDrawn = currentWorld.renderChunksAround(currentCam.getPosition().x, currentCam.getPosition().z, waterTexCoords);
                //currentWorld.renderChunksAround(-18, -8);
                chunksDrawn = currentWorld.render(chunksToRender, waterTexCoords);
                break;
#ifdef OPTIMIZATION_FRUSTUM_CULLING
            case RenderChunkMode::CHUNKS_IN_FRUSTUM:
                //OLD METHOD:
                //chunksDrawn = currentWorld.renderChunksInFrustum(frustum, waterTexCoords);
                chunksDrawn = currentWorld.render(chunksToRender, waterTexCoords);
                break;
#endif
        }

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        if(options.lightning) Renderer::SetLightOff();
#endif
        
        if(options.boundingBox) {
            GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
            // Renderer::PrepareToRenderInVX2(true, false, true, false);
            // auto& chunkitos = currentWorld.getChunks();
            // for(const auto& chunkito : chunkitos) {
            //     Renderer::RenderBoundingBox(chunkito->worldPosition_.x, 0, chunkito->worldPosition_.z, CHUNK_SIZE, CHUNK_HEIGHT, UCVec3{0, 255, 255}, true);
            // }
            Renderer::PrepareToRenderInVX0(true, false, true, false);
            currentWorld.renderChunksBoundings();
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

            //Screen Things
            text.render(USVec2{5, 170}, fmt::format("Video Mode : {}", std::array{"INTERLACE", "NON INTERLACE", "PROGRESSIVE", "ERROR"}[static_cast<int>(Renderer::VideoMode())]).c_str());
            text.render(USVec2{5, 185}, fmt::format("VSYNC      : {}", options.VSYNC ? "YES" : "NOP").c_str());
        	text.render(USVec2{5, 200}, fmt::format("Resolution  : {}/{}", Renderer::ScreenWidth(), Renderer::ScreenHeight()).c_str());
#ifdef ENABLE_MEASUREMENTS
            text.render(USVec2{5, 215}, fmt::format("File  : {}", fileName).c_str());
#endif
            String ChunkRenderMode = "DEFAULT";
            switch (options.chunkDrawMode) {
                case RenderChunkMode::CHUNKS_AROUND:        ChunkRenderMode = "AROUND THE CAMERA"; break;
            #ifdef OPTIMIZATION_FRUSTUM_CULLING
                case RenderChunkMode::CHUNKS_IN_FRUSTUM:    ChunkRenderMode = "FRUSTUM CULLED";    break;
            #endif
            }
            
            //Camera Things
            text.render(USVec2{275,  5}, fmt::format("Camera X [{:.4f}] Y [{:.4f}] Z [{:.4f}]", camPos.x, camPos.y, camPos.z).c_str());
            text.render(USVec2{275, 20}, fmt::format("Camera Pitch [{:.4f}] Yaw [{:.4f}]", currentCam.getPitch(), currentCam.getYaw()).c_str());

            //Render Things
            text.render(USVec2{275,  35}, fmt::format("Valid: Blocks [{}] Faces [{}]", currentWorld.validBlocks_, currentWorld.validFaces_).c_str());
            text.render(USVec2{275,  50}, fmt::format("Calls : {} Draws / {} Faces", Renderer::DrawCalls(), Renderer::FacesDrawn()).c_str());
            //text.render(USVec2{275,  80}, fmt::format("NFaces Drawn : {}", Renderer::FacesDrawn()).c_str());  //todo: fix
            text.render(USVec2{275,  65}, fmt::format("NChunks     : [{}]/[{}]", chunksDrawn, currentWorld.NChunks()).c_str());
            text.render(USVec2{485, 65}, fmt::format("NTrees : {}", currentWorld.nTrees_).c_str());
#ifdef KIRBY_EASTER_EGG
            text.render(USVec2{485, 80}, fmt::format("NKirby : {}", currentWorld.NKirbys).c_str()); //currentWorld.nTrees_
#endif
            text.render(USVec2{295,  95}, fmt::format("Draw   : Cycles {} ts / Time {} ms", drawTicks, Tick::TickToMsfloat(drawTicks)).c_str());
            text.render(USVec2{295, 110}, fmt::format("Frame  : Cycles {} ts", frameTicks).c_str());
            text.render(USVec2{295, 125}, fmt::format("MODE   : {}", ChunkRenderMode).c_str());
            text.render(USVec2{295, 140}, fmt::format("Helper : {}", helpValue).c_str());
            //text.render(USVec2{400,  95}, fmt::format("Helper      : {}", currentWorld.helperCounter).c_str());
            //text.render(USVec2{400, 110}, fmt::format("N Blocks    : {}", currentChunk.validBlocks).c_str());
        }

#ifdef ENABLE_MEASUREMENTS
        if(measurement_system.shouldRecordFrame()) {
            MeasurementsByFrame frame;
            frame.frameTimeInMs = deltaTime * 1000.0f; //s to ms
            frame.DrawTimeInMs  = Tick::TickToMsfloat(drawTicks);
            frame.NChunksDrawn  = chunksDrawn;
            frame.NDrawCalls    = Renderer::DrawCalls();
            frame.NFacesDrawn   = Renderer::FacesDrawn();
            measurement_system.recordFrame(frame);
            if (measurement_system.isMeasurementComplete()) {
                if constexpr (MEASUREMENTS_FILE_FORMAT == FileFormat::TXT) {
                    measurement_system.dumpToFile(fmt::format("{}.txt", fileName));
                }else if constexpr (MEASUREMENTS_FILE_FORMAT == FileFormat::CSV) {
                    measurement_system.dumpToCSV(fmt::format("{}.csv", fileName));
                }
                helpValue = "Recording Done!";
            }
        }

#endif
        
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

