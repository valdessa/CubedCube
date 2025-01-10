//Blocks TPL data

#include "bloquitos_tpl.h"
#include "bloquitos.h"

// Font
#include "Karma_ttf.h"

static TPLFile bloquitosTPL;
static GXTexObj blocksTexture;

static void loadResources() {
#ifdef OPTIMIZATION_FRUSTUM_CULLING
    CHUNK_LOAD_RADIUS = CHUNK_RADIUS;
#endif
    
    TPL_OpenTPLFromMemory(&bloquitosTPL, (void*)bloquitos_tpl, bloquitos_tpl_size);
    TPL_GetTexture(&bloquitosTPL, blocksTextureID, &blocksTexture);
    GX_InitTexObjWrapMode(&blocksTexture, GX_CLAMP, GX_CLAMP);
    GX_InitTexObjFilterMode(&blocksTexture, GX_NEAR, GX_NEAR);
    GX_InitTexObjLOD(&blocksTexture, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GX_InvalidateTexAll();
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

inline bool updateInput(Options& options, Camera& cam) {
    PAD_ScanPads(); // Scan the GameCube controllers

    // If [START/PAUSE] was pressed on the first GameCube controller, break out of the loop
    if(PAD_ButtonsDown(0) & PAD_BUTTON_START) return true;
    if(PAD_ButtonsDown(0) & PAD_TRIGGER_Z) cam.setPosition(FVec3(0));
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
    if(PAD_ButtonsDown(0) & PAD_BUTTON_A) {
        int nextMode = (static_cast<int>(options.chunkDrawMode) + 1) % static_cast<int>(RenderChunkMode::COUNT);
        options.chunkDrawMode = static_cast<RenderChunkMode>(nextMode);
        //options.chunksAround = !options.chunksAround;
    }
    if(PAD_ButtonsDown(0) & PAD_TRIGGER_R) cam.setSpeed(CameraSpeed * 5.0f);
    if(PAD_ButtonsUp(0) & PAD_TRIGGER_R) cam.setSpeed(CameraSpeed);

    return false;
}

u8 waterTexCoords[8];
int textureCounter = 0;
float textureCounterFloat = 0.0f;
constexpr float TextureTime = 0.33f;
static void updateWaterTextureCoordinates(float deltaTime, u8 offset = 5) {
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
    // Calculamos el desplazamiento base de las coordenadas en función de textureCounter
    u8 baseOffset = static_cast<u8>(textureCounter / 4);  // Los niveles cambian en 4, es decir, 0, 4, 8, 12, ...

    waterTexCoords[0] = 0 + baseOffset;     waterTexCoords[1] = 0 + offset;
    waterTexCoords[2] = 1 + baseOffset;     waterTexCoords[3] = 0 + offset;
    waterTexCoords[4] = 1 + baseOffset;     waterTexCoords[5] = 1 + offset;
    waterTexCoords[6] = 0 + baseOffset;     waterTexCoords[7] = 1 + offset;
}

static void fillCubitoStatic(Cubito& cubito, U8 face, U8 direction, S32 block) {
    auto& currentFace = cubito.face[face];
    currentFace.direction = direction;
    if(direction != DIR_DIAG_XY_BACK && direction != DIR_DIAG_XY_FRONT) {
        currentFace.tile = blockTiles[block][direction];
    }else if(direction == DIR_DIAG_XY_FRONT) {
        currentFace.tile = blockTiles[block][0];
    }else if(direction == DIR_DIAG_XY_BACK) {
        currentFace.tile = blockTiles[block][1];
    }
}

static void setCubitoStatic(Cubito& cubito, const CubePosition& pos, BLOCK_TYPE block) {
    cubito.x = pos.x;
    cubito.y = pos.y;
    cubito.z = pos.z;
    cubito.type = block;

    if(hasProperty(block, FOLIAGE)) {
        fillCubitoStatic(cubito, 0, DIR_DIAG_XY_FRONT, block);  // Front
        fillCubitoStatic(cubito, 1, DIR_DIAG_XY_BACK,  block);  // Back
        fillCubitoStatic(cubito, 2, DIR_Y_FRONT, BLOCK_AIR);  // Top
        fillCubitoStatic(cubito, 3, DIR_Y_BACK,  BLOCK_AIR);  // Bottom
        fillCubitoStatic(cubito, 4, DIR_Z_FRONT, BLOCK_AIR);  // Left
        fillCubitoStatic(cubito, 5, DIR_Z_BACK,  BLOCK_AIR);  // Right
    }else {
        fillCubitoStatic(cubito, 0, DIR_X_FRONT, block);  // Front
        fillCubitoStatic(cubito, 1, DIR_X_BACK,  block);  // Back
        fillCubitoStatic(cubito, 2, DIR_Y_FRONT, block);  // Top
        fillCubitoStatic(cubito, 3, DIR_Y_BACK,  block);  // Bottom
        fillCubitoStatic(cubito, 4, DIR_Z_FRONT, block);  // Left
        fillCubitoStatic(cubito, 5, DIR_Z_BACK,  block);  // Right
    }
}