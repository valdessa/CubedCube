#include <common.h>
#include <ogc/gx.h>
#include <renderer.h>
#include <utilities.h>
#include <ogc/system.h>
#include <ogc/video.h>

//memalign & memset:
#include <malloc.h> 
#include <string.h>

//#include <grrlib.h>


using namespace poyo;

extern U16 vertices[8][3];
extern U16 edges[12][2];
extern U16 diagonals[12][2];
extern u16 tileTexCoords[4][2];

//GX Stuff
GXRModeObj* videoMode = nullptr;
void* frameBuffers[2] = {nullptr, nullptr};
u32 currentFrameBuffer = 0;
void* gpFifo = nullptr;
#define DEFAULT_FIFO_SIZE (256 * 1024) /**< GX fifo buffer size. */

static int gDisplayWidth = 0;
static int gDisplayHeight = 0;
static Mtx                 GXmodelView2D;

int Renderer::InitializeGX() {
    Mtx44 perspective;
    // Initialise the video subsystem
    VIDEO_Init();

    videoMode = VIDEO_GetPreferredMode(nullptr);
    gDisplayWidth = videoMode->fbWidth;
    gDisplayHeight = videoMode->efbHeight;
    if(videoMode == nullptr) return -1;
    //videoMode = &TVNtsc480Prog;
    // Video Mode Correction
    switch (videoMode->viTVMode) {
    case VI_DEBUG_PAL:  // PAL 50hz 576i
        //rmode = &TVPal574IntDfScale;
            videoMode = &TVPal528IntDf; // BC ...this is still wrong, but "less bad" for now
        break;
    default:
#ifdef HW_DOL
        if(VIDEO_HaveComponentCable()) {
            videoMode = &TVNtsc480Prog;
        }
#endif
        break;
    }
    
    VIDEO_Configure(videoMode);

    // Get some memory to use for a "double buffered" frame buffer
    frameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(videoMode));
    frameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(videoMode));
    if (!frameBuffers[0]) return -1;
    if (!frameBuffers[1]) return -1;

    // Choose a frame buffer to start with
    VIDEO_SetNextFramebuffer(frameBuffers[currentFrameBuffer]);  
    VIDEO_Flush();                      // flush the frame to the TV
    VIDEO_WaitVSync();                  // Wait for the TV to finish updating

    gpFifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gpFifo, 0, DEFAULT_FIFO_SIZE);
    GX_Init(gpFifo, DEFAULT_FIFO_SIZE);

    // Clear the background to opaque black and clears the z-buffer
    GX_SetCopyClear((GXColor){ 0, 0, 0, 0 }, GX_MAX_Z24);

    //todo: fix this
    //GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);  // Set 16 bit RGB565
    if (videoMode->aa) {
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);  // Set 16 bit RGB565
    }
    else {
        GX_SetPixelFmt(GX_PF_RGB8_Z24  , GX_ZC_LINEAR);  // Set 24 bit Z24
    }

    // Other GX setup
    f32 yscale    = GX_GetYScaleFactor(videoMode->efbHeight, videoMode->xfbHeight);
    u32 xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetDispCopySrc(0, 0, videoMode->fbWidth, videoMode->efbHeight);
    GX_SetDispCopyDst(videoMode->fbWidth, xfbHeight);
    GX_SetCopyFilter(videoMode->aa, videoMode->sample_pattern, GX_TRUE, videoMode->vfilter);
    GX_SetFieldMode(videoMode->field_rendering, ((videoMode->viHeight == 2 * videoMode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

    GX_CopyDisp(frameBuffers[currentFrameBuffer], GX_TRUE);  //Draw first frame
    GX_SetDispCopyGamma(GX_GM_1_0);
    
    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);

    GX_SetNumChans(1);    // colour is the same as vertex colour
    GX_SetNumTexGens(1);  // One texture exists
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    guMtxIdentity(GXmodelView2D);
    guMtxTransApply(GXmodelView2D, GXmodelView2D, 0.0f, 0.0f, -100.0f);
    GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);
    
    guOrtho(perspective, 0.0f, videoMode->efbHeight, 0.0f, videoMode->fbWidth, 0.0f, 1000.0f);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

    GX_SetViewport(0.0f, 0.0f, videoMode->fbWidth, videoMode->efbHeight, 0.0f, 1.0f);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GX_SetColorUpdate(GX_ENABLE);
    GX_SetCullMode(GX_CULL_NONE);
    
    GX_SetClipMode( GX_CLIP_ENABLE );
    GX_SetScissor( 0, 0, videoMode->fbWidth, videoMode->efbHeight );

    VIDEO_SetBlack(false);  // Enable video output
    
    return 0;
}

void Renderer::Initialize() {
    mapTileUVs(6);
    
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT); 
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT); 
    
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_U16, 0);     //Positions -> U16
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);     //Normals   -> S8
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0); //Color     -> UChar
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);     //Textures  -> U16
}

static  guVector  _GRRaxisx = (guVector){1.0f, 0.0f, 0.0f}; // DO NOT MODIFY!!!
static  guVector  _GRRaxisy = (guVector){0.0f, 1.0f, 0.0f}; // Even at runtime
static  guVector  _GRRaxisz = (guVector){0.0f, 0.0f, 1.0f}; // NOT ever!
static  Mtx       _GRR_view; 

static  guVector camPos, camUp, camLook;

void Renderer::Set3DMode(Camera& cam) {
    Mtx44 projectionMtx;
    //
    guLookAt(_GRR_view, &camPos, &camUp, &camLook);
    guPerspective(projectionMtx, 60, (float)gDisplayWidth / (float)gDisplayHeight, 0.1f, 300.0f);
    GX_LoadProjectionMtx(projectionMtx, GX_PERSPECTIVE);

    GX_SetZMode (GX_TRUE, GX_LEQUAL, GX_TRUE);

    GX_SetCullMode(GX_CULL_NONE);

}

void Renderer::Set2DMode() {
    // Mtx44 projectionMtx;
    // Mtx positionMtx;
    //
    // guOrtho(projectionMtx, 0, gDisplayHeight, 0, gDisplayWidth, 0.0, 1.0);
    // GX_LoadProjectionMtx(projectionMtx, GX_ORTHOGRAPHIC);
    // guMtxIdentity(positionMtx);
    // GX_LoadPosMtxImm(positionMtx, GX_PNMTX0);
    
    Mtx view;
    Mtx44 m;

    GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);

    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    guOrtho(m, 0, videoMode->efbHeight, 0, videoMode->fbWidth, 0, 1000.0f);
    GX_LoadProjectionMtx(m, GX_ORTHOGRAPHIC);

    guMtxIdentity(view);
    guMtxTransApply(view, view, 0, 0, -100.0f);
    GX_LoadPosMtxImm(view, GX_PNMTX0);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GX_SetNumTexGens(1);  // One texture exists
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    GX_SetNumTevStages(1);

    GX_SetTevOp  (GX_TEVSTAGE0, GX_PASSCLR);

    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}

static U32 nFacesRendered = 0;
static U32 nDrawCalls = 0;
void Renderer::ResetDrawCalls() {
    nFacesRendered = 0;
    nDrawCalls = 0;
}

U32 Renderer::DrawCalls() {
    return nDrawCalls;
}

U32 Renderer::FacesDrawn() {
    return nFacesRendered;
}

VIDEO_MODE Renderer::VideoMode() {
    switch (videoMode->viTVMode) {
        case 0: return VIDEO_MODE::INTERLACE;
        case 1: return VIDEO_MODE::NON_INTERLACE;
        case 2: return VIDEO_MODE::PROGRESSIVE;
    }
    return VIDEO_MODE::VIDEO_ERROR;
}

void Renderer::ObjectView(f32 posx, f32 posy, f32 posz, f32 angx, f32 angy, f32 angz, f32 scalx, f32 scaly, f32 scalz) {
    // GRRLIB_ObjectView(posx, posy, posz,
    //                   angx, angy, angz,
    //                   scalx, scaly, scalz);
    Mtx ObjTransformationMtx;
    Mtx m;
    Mtx mv, mvi;

    guMtxIdentity(ObjTransformationMtx);

    if((scalx != 1.0f) || (scaly != 1.0f) || (scalz != 1.0f)) {
        guMtxIdentity(m);
        guMtxScaleApply(m, m, scalx, scaly, scalz);

        guMtxConcat(m, ObjTransformationMtx, ObjTransformationMtx);
    }

    if((angx != 0.0f) || (angy != 0.0f) || (angz != 0.0f)) {
        Mtx rx, ry, rz;
        guMtxIdentity(m);
        guMtxRotAxisDeg(rx, &_GRRaxisx, angx);
        guMtxRotAxisDeg(ry, &_GRRaxisy, angy);
        guMtxRotAxisDeg(rz, &_GRRaxisz, angz);
        guMtxConcat(ry, rx, m);
        guMtxConcat(m, rz, m);

        guMtxConcat(m, ObjTransformationMtx, ObjTransformationMtx);
    }

    if((posx != 0.0f) || (posy != 0.0f) || (posz != 0.0f)) {
        guMtxIdentity(m);
        guMtxTransApply(m, m, posx, posy, posz);

        guMtxConcat(m, ObjTransformationMtx, ObjTransformationMtx);
    }

    guMtxConcat(_GRR_view, ObjTransformationMtx, mv);
    GX_LoadPosMtxImm(mv, GX_PNMTX0);

    guMtxInverse(mv, mvi);
    guMtxTranspose(mvi, mv);
    GX_LoadNrmMtxImm(mv, GX_PNMTX0);
}

void Renderer::SetTextureCoordScaling(U8 unit, U16 scaleX, U16 scaleY) {
    GX_SetTexCoordScaleManually(unit, GX_TRUE, scaleX, scaleY); //GX_TEXCOORD0
}

void Renderer::SetBackgroundColour(const u8 r, const u8 g, const u8 b, const u8 a) {
    GX_SetCopyClear(GXColor{ r, g, b, a }, GX_MAX_Z24);
}

void Renderer::SetCameraSettings(cFVec3& position, cFVec3& up, cFVec3& look) {
    camPos.x = position.x;
    camPos.y = position.y;
    camPos.z = position.z;

    camUp.x = up.x;
    camUp.y = up.y;
    camUp.z = up.z;

    camLook.x = look.x;
    camLook.y = look.y;
    camLook.z = look.z;
}

void Renderer::EnableBlend(const BLEND_MODE mode) {
    //GX_SetAlphaUpdate(GX_TRUE);
    //GX_SetAlphaCompare(GX_GREATER, 128, GX_AOP_AND, GX_ALWAYS, 200);
    switch (mode) {
        case BLEND_MODE::ALPHA:  GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
            break;
        case BLEND_MODE::ADD:    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_DSTALPHA, GX_LO_CLEAR);
            break;
        case BLEND_MODE::SCREEN: GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCCLR, GX_BL_DSTALPHA, GX_LO_CLEAR);
            break;
        case BLEND_MODE::MULTI:  GX_SetBlendMode(GX_BM_SUBTRACT, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
            break;
        case BLEND_MODE::INV:    GX_SetBlendMode(GX_BM_BLEND, GX_BL_INVSRCCLR, GX_BL_INVSRCCLR, GX_LO_CLEAR);
            break;
    }
}

void Renderer::DisableBlend() {
    GX_SetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
}

void Renderer::BindTexture(GXTexObj& obj, U8 unit) {
    GX_LoadTexObj(&obj, unit); //GX_TEXMAP0
}

void Renderer::PrepareToRender(bool pos, bool nrm, bool clr, bool tex) {
    GX_ClearVtxDesc();
    
    if(pos)     GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    if(nrm)     GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT); 
    if(clr)     GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    if(tex)     GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT); 
    
    if(pos)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_U16, 0);     //Positions -> U16
    if(nrm)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);      //Normals   -> S8
    if(clr)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0); //Color     -> UChar
    if(tex)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);     //Textures  -> U16

    tex ? GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE) : GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

void Renderer::RenderBegin(U16 VertexCount) {
    nDrawCalls++;
    GX_Begin(GX_QUADS, GX_VTXFMT2, VertexCount);
}

void Renderer::RenderEnd() {
    GX_End();
}

void Renderer::RenderFace(const CubeFace& face) {
    nFacesRendered++;
    for (int j = 0; j < 4; j++) {
        GX_Position3u16(face.x + cubeFaces[face.direction][j][0],
                        face.y + cubeFaces[face.direction][j][1],
                        face.z + cubeFaces[face.direction][j][2]);
        GX_Normal3s8(cubeNormals[face.direction][0],
                     cubeNormals[face.direction][1],
                     cubeNormals[face.direction][2]);
        GX_Color4u8(255, 255, 255, 255);

        auto& UV = tileUVMap[face.tile];

#ifdef OPTIMIZATION_MAPS
        GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#else
        GX_TexCoord2u16(UV.x + tileTexCoords[j][0], UV.y + tileTexCoords[j][1]);
#endif
    }
    GX_End();
}

void Renderer::RenderFace(const CubeFace& face, S8 x, S8 y, S8 z) {
    nFacesRendered++;
    for (int j = 0; j < 4; j++) {
        GX_Position3u16(face.x + cubeFaces[face.direction][j][0] + x,
                        face.y + cubeFaces[face.direction][j][1] + y,
                        face.z + cubeFaces[face.direction][j][2] + z);
        GX_Normal3s8(cubeNormals[face.direction][0],
                     cubeNormals[face.direction][1],
                     cubeNormals[face.direction][2]);
        GX_Color4u8(255, 255, 255, 255);

        auto& UV = tileUVMap[face.tile];

#ifdef OPTIMIZATION_MAPS
        GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#else
        GX_TexCoord2u16(UV.x + tileTexCoords[j][0], UV.y + tileTexCoords[j][1]);
#endif
    }
    GX_End();
}

void Renderer::RenderCube(const Cubito& cube, cFVec3& worldPos, cFVec3& angle) {
    cFVec3 position = cFVec3(static_cast<float>(cube.x) + worldPos.x,
                             static_cast<float>(cube.y)  + worldPos.y,
                             static_cast<float>(cube.z)  + worldPos.z);
    ObjectView(position.x, position.y, position.z,
                      angle.x, angle.y, angle.z,
                      1.0f, 1.0f, 1.0f);

    RenderBegin(24);
    for (auto& currentFace : cube.face) {
        RenderFace(currentFace);
    }
    RenderEnd();
}

void Renderer::RenderCubeVector(const Vector<Cubito>& cubes, U16 validBlocks) {
    RenderBegin(validBlocks);

    for(auto& cubito: cubes) {
        if(!cubito.visible) continue;
        for (auto& currentFace : cubito.face) {
            RenderFace(currentFace, cubito.x, cubito.y, cubito.z);
        }
    }

    RenderEnd();
}

void Renderer::RenderFaceVector(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks) {
    GX_Begin(GX_QUADS, GX_VTXFMT2, validBlocks);
    for (auto& currentFaceVec : faces) {
        auto& currentFace = currentFaceVec.first;
#ifdef OPTIMIZATION_BATCHING
        auto& cubito = currentFaceVec.second;
        RenderFace(currentFace, cubito.x, cubito.y, cubito.z);
#else
        RenderFace(currentFace);
#endif
    }

    GX_End();
}

void Renderer::RenderBoundingBox(S16 originX, S16 originY, S16 originZ, U16 size, cUCVec3& color, bool RenderCross) {
    // Set the view for the cube based on origin and size
    //GRRLIB_ObjectView(originX, originY, originZ, 0, 0, 0.0f, size, size, size);
    ObjectView(originX, originY, originZ, 0, 0, 0.0f, size, size, size);

    GX_Begin(GX_LINES, GX_VTXFMT2, RenderCross ? 48 : 24);

    // Draw the edges of the cube
    for (const auto& edge : edges) {
        GX_Position3u16(vertices[edge[0]][0], vertices[edge[0]][1], vertices[edge[0]][2]);
        GX_Color4u8(color.x, color.y, color.z, 255);
        
        GX_Position3u16(vertices[edge[1]][0], vertices[edge[1]][1], vertices[edge[1]][2]);
        GX_Color4u8(color.x, color.y, color.z, 255);
    }

    // Render cross if specified
    if(RenderCross) {
        // Draw the diagonals only on the faces
        for (const auto& diagonal : diagonals) {
            GX_Position3u16(vertices[diagonal[0]][0], vertices[diagonal[0]][1], vertices[diagonal[0]][2]);
            GX_Color4u8(color.x, color.y, color.z, 255);
    
            GX_Position3u16(vertices[diagonal[1]][0], vertices[diagonal[1]][1], vertices[diagonal[1]][2]);
            GX_Color4u8(color.x, color.y, color.z, 255);
        }
    }
    
    GX_End();
}

void Renderer::CallDisplayList(void* list, U32 size) {
    nDrawCalls++;
    GX_CallDispList(list, size);
}

void Renderer::RenderGX(bool VSYNC) {
    GX_DrawDone();         
    //GX_InvalidateTexAll();

    currentFrameBuffer ^= 1;  // Toggle framebuffer index

    GX_SetZMode      (GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_CopyDisp      (frameBuffers[currentFrameBuffer], GX_TRUE);
    
    // GX_SetDrawDone();
    //
    // if(!VSYNC) {
    //     GX_WaitDrawDone();
    //     VIDEO_SetNextFramebuffer(frameBuffers[currentFrameBuffer]);
    //     VIDEO_Flush();
    // }
    
    VIDEO_SetNextFramebuffer(frameBuffers[currentFrameBuffer]);  
    VIDEO_Flush();                      // Flush video buffer to screen
    VIDEO_WaitVSync();                  // Wait for screen to update
    
    // Interlaced screens require two frames to update
    if (videoMode->viTVMode &VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }
}

u16 tileTexCoords[4][2] = {
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1}
};

// Cube vertices
U16 vertices[8][3] = {
    {0, 0, 0},  // Vertex 0: bottom left corner (origin)
    {0, 0, 1},  // Vertex 1: bottom right corner (Z positive)
    {0, 1, 0},  // Vertex 2: top left corner (Y positive)
    {0, 1, 1},  // Vertex 3: top right corner (Y positive, Z positive)
    {1, 0, 0},  // Vertex 4: bottom left corner (X positive)
    {1, 0, 1},  // Vertex 5: bottom right corner (X and Z positive)
    {1, 1, 0},  // Vertex 6: top left corner (X and Y positive)
    {1, 1, 1}   // Vertex 7: top right corner complete (X, Y, Z positive)
};

// Pairs of indices that form the edges of the cube
U16 edges[12][2] = {
    {0, 1}, {1, 3}, {3, 2}, {2, 0},  // Front face
    {4, 5}, {5, 7}, {7, 6}, {6, 4},  // Back face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Edges connecting front and back faces
};

// Pairs of indices for the diagonals on the six faces
U16 diagonals[12][2] = {
    // Diagonals of the front face 0
    {0, 3}, {1, 2},  
    // Diagonals of the back face 2
    {4, 7}, {5, 6},  
    // Diagonals of the left face 4
    {0, 6}, {2, 4},  
    // Diagonals of the right face 6
    {1, 7}, {3, 5},  
    // Diagonals of the top face 8
    {2, 7}, {3, 6},  
    // Diagonals of the bottom face 10
    {0, 5}, {1, 4}   
};


//Render Line
// GRRLIB_ObjectView(0, 0, 0, 0, 0, 0.0f, 1.0f, 1.0f, 1.0f);
// GX_Begin(GX_LINES, GX_VTXFMT3, 2);
// GX_Position2u16(0, 0);
// GX_Color4u8(255, 0, 0, 255);
// GX_Position2u16(0, 10);
// GX_Color4u8(255, 0, 0, 255);
// GX_End();

//291