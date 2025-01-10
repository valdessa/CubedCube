#include <common.h>
#include <ogc/gx.h>
#include <renderer.h>
#include <utilities.h>
#include <ogc/system.h>
#include <ogc/video.h>

//memalign & memset:
#include <malloc.h> 
#include <string.h>

#include "camera.h"

using namespace poyo;

extern U16 vertices[8][3];
extern U16 edges[12][2];
extern U16 diagonals[12][2];
extern u16 tileTexCoords[4][2];
extern U8  positionsCoords[6][3];

//GX Stuff
GXRModeObj* videoMode = nullptr;
void* frameBuffers[2] = {nullptr, nullptr};
u32 currentFrameBuffer = 0;
void* gpFifo = nullptr;
constexpr U32 DEFAULT_FIFO_SIZE (256 * 1024); /**< GX fifo buffer size. */

static int gDisplayWidth = 0;
static int gDisplayHeight = 0;
static Mtx                 GXmodelView2D;

//Transform things:
static guVector AxisX = {1.0f, 0.0f, 0.0f}; 
static guVector AxisY = {0.0f, 1.0f, 0.0f}; 
static guVector AxisZ = {0.0f, 0.0f, 1.0f}; 
static Mtx      viewMatrix;

static  guVector camPos, camUp, camLook;

static U8 LastDepthMode = GX_LEQUAL;
static bool antialiased = false;

//Light Things:
static int lights = 0;

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
        antialiased = true;
        GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);  // Set 16 bit RGB565
    }
    else {
        antialiased = false;
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

    gDisplayWidth = videoMode->fbWidth;
    gDisplayHeight = videoMode->efbHeight;
    return 0;
}

static void drawDoneCallback() {
    VIDEO_SetNextFramebuffer(frameBuffers[currentFrameBuffer]);
    VIDEO_Flush();
    currentFrameBuffer ^= 1;
}

void Renderer::Initialize() {
    mapTileUVs(8);
    
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT); 
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT); 
    
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_U16, 0);     //Positions -> U16
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);      //Normals   -> S8
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0); //Color     -> UChar
    GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);     //Textures  -> U16

    GX_SetDrawDoneCallback(drawDoneCallback);
}

void Renderer::Exit() {
    GX_DrawDone();
    GX_AbortFrame();

    // Free up memory allocated for frame buffers & FIFOs
    if (frameBuffers[0] != nullptr) {
        free(MEM_K1_TO_K0(frameBuffers[0]));
        frameBuffers[0] = nullptr;
    }
    if (frameBuffers[1] != nullptr) {
        free(MEM_K1_TO_K0(frameBuffers[1]));
        frameBuffers[1] = nullptr;
    }
    if (gpFifo != nullptr) {
        free(gpFifo);
        gpFifo = nullptr;
    }
}

void Renderer::Set3DMode(const Camera& cam) {
    Mtx44 projectionMtx;
    //
    guLookAt(viewMatrix, &camPos, &camUp, &camLook);
    guPerspective(projectionMtx, cam.fov_, cam.aspectRatio_, cam.near_, cam.far_);
    GX_LoadProjectionMtx(projectionMtx, GX_PERSPECTIVE);
    
    SetDepth(GX_TRUE, DEPTH_MODE::LEQUAL, GX_TRUE);

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

    SetDepth(GX_FALSE, DEPTH_MODE::LEQUAL, GX_TRUE);

    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    
    guOrtho(m, 0, videoMode->efbHeight, 0, videoMode->fbWidth, 0, 1000.0f);
    GX_LoadProjectionMtx(m, GX_ORTHOGRAPHIC);

    guMtxIdentity(view);
    guMtxTransApply(view, view, 0, 0, -100.0f);
    GX_LoadPosMtxImm(view, GX_PNMTX0);

    // GX_ClearVtxDesc();
    // GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    // GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    // GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
    // GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    // GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    // GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

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

void Renderer::AddToFacesDrawn(U32 facesDrawn) {
    nFacesRendered += facesDrawn;
}

bool Renderer::isAntialiased() {
    return antialiased;
}

int Renderer::ScreenWidth() {
    return gDisplayWidth;
}

int Renderer::ScreenHeight() {
    return gDisplayHeight;
}

VIDEO_MODE Renderer::VideoMode() {
    switch (VIDEO_GetVideoScanMode()) {
        case 0: return VIDEO_MODE::INTERLACE;
        case 1: return VIDEO_MODE::NON_INTERLACE;
        case 2: return VIDEO_MODE::PROGRESSIVE;
    }
    return VIDEO_MODE::VIDEO_ERROR;
}

const Mtx& Renderer::ViewMatrix() {
    return viewMatrix;
}

void Renderer::CalculateModelMatrix(Mtx& modelToFill, const Transform& trans) {
    guMtxIdentity(modelToFill);
    guMtxScaleApply(modelToFill, modelToFill, trans.Scale.x, trans.Scale.y, trans.Scale.z);

    Mtx m;
    Mtx rx, ry, rz;
    guMtxIdentity(m);
    guMtxRotAxisDeg(rx, &AxisX, trans.Rotation.x);
    guMtxRotAxisDeg(ry, &AxisY, trans.Rotation.y);
    guMtxRotAxisDeg(rz, &AxisZ, trans.Rotation.z);
    guMtxConcat(ry, rx, m);
    guMtxConcat(m, rz, m);

    guMtxConcat(m, modelToFill, modelToFill);
    
    guMtxTransApply(modelToFill, modelToFill, trans.Position.x, trans.Position.y, trans.Position.z);
}

void Renderer::CalculateModelMatrix(Mtx& modelToFill, f32 posx, f32 posy, f32 posz) {
    guMtxIdentity(modelToFill);

    guMtxTransApply(modelToFill, modelToFill, posx, posy, posz);
}

void Renderer::ObjectView() {
    GX_LoadPosMtxImm(viewMatrix, GX_PNMTX0);
}

void Renderer::ObjectView(const Transform& trans) {
    ObjectView(trans.Position.x, trans.Position.y, trans.Position.z,
        trans.Rotation.x, trans.Rotation.y, trans.Rotation.z,
        trans.Scale.x, trans.Scale.y, trans.Scale.z);
}

void Renderer::ObjectView(f32 posx, f32 posy, f32 posz, f32 angx, f32 angy, f32 angz, f32 scalx, f32 scaly, f32 scalz) {
    Mtx modelMatrix;
    Mtx m;
    Mtx modelViewMatrix, ModelViewInverse;
    
    guMtxIdentity(modelMatrix);

    if((scalx != 1.0f) || (scaly != 1.0f) || (scalz != 1.0f)) {
        guMtxIdentity(m);
        guMtxScaleApply(m, m, scalx, scaly, scalz);

        guMtxConcat(m, modelMatrix, modelMatrix);
    }

    if((angx != 0.0f) || (angy != 0.0f) || (angz != 0.0f)) {
        Mtx rx, ry, rz;
        guMtxIdentity(m);
        guMtxRotAxisDeg(rx, &AxisX, angx);
        guMtxRotAxisDeg(ry, &AxisY, angy);
        guMtxRotAxisDeg(rz, &AxisZ, angz);
        guMtxConcat(ry, rx, m);
        guMtxConcat(m, rz, m);

        guMtxConcat(m, modelMatrix, modelMatrix);
    }

    if((posx != 0.0f) || (posy != 0.0f) || (posz != 0.0f)) {
        guMtxIdentity(m);
        guMtxTransApply(m, m, posx, posy, posz);

        guMtxConcat(m, modelMatrix, modelMatrix);
    }

    guMtxConcat(viewMatrix, modelMatrix, modelViewMatrix);
    GX_LoadPosMtxImm(modelViewMatrix, GX_PNMTX0);

    guMtxInverse(modelViewMatrix, ModelViewInverse);
    guMtxTranspose(ModelViewInverse, modelViewMatrix);
    GX_LoadNrmMtxImm(modelViewMatrix, GX_PNMTX0);
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

void Renderer::SetCullFace(const CULL_MODE mode) {
    switch (mode) {
        case CULL_MODE::MODE_NONE:  GX_SetCullMode(GX_CULL_NONE);
            break;
        case CULL_MODE::MODE_BACK:  GX_SetCullMode(GX_CULL_BACK);
            break;
        case CULL_MODE::MODE_FRONT: GX_SetCullMode(GX_CULL_FRONT);
            break;
        case CULL_MODE::MODE_ALL:   GX_SetCullMode(GX_CULL_ALL);
            break;
        default: ;
    }
}

void Renderer::SetBlend(const BLEND_MODE mode) {
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
        case BLEND_MODE::MODE_OFF: GX_SetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ONE, GX_LO_CLEAR);
                                   //GX_SetAlphaUpdate(GX_FALSE);
            break;
        default: ;
    }
}

void Renderer::SetDepth(bool enable, DEPTH_MODE mode, bool update) {
    switch(mode) {
        case DEPTH_MODE::LESS:      LastDepthMode = GX_LESS;    break;
        case DEPTH_MODE::LEQUAL:    LastDepthMode = GX_LEQUAL;  break;
        case DEPTH_MODE::EQUAL:     LastDepthMode = GX_EQUAL;   break;
        default: ;
    }
    GX_SetZMode(enable, LastDepthMode, update);
}

void Renderer::EnableFog() {
    GX_SetFog(GX_FOG_LIN, 20.0f, 30.0f, 0.1f, 1000.0f, GXColor{192, 216, 255, 0});
}

void Renderer::DisableFog() {
    GX_SetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, { 0, 0, 0, 0 });
}

void Renderer::SetAlphaTest(bool enable) {
    if(enable) {
        GX_SetAlphaCompare(GX_GEQUAL, 16, GX_AOP_AND, GX_ALWAYS, 0);
        GX_SetZCompLoc(GX_FALSE);
    } else {
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZCompLoc(GX_TRUE);
    }
}

void Renderer::BindTexture(GXTexObj& obj, U8 unit) {
    GX_LoadTexObj(&obj, unit); //GX_TEXMAP0
}

void Renderer::SetLightOff() {
    GX_SetNumTevStages(1);

    GX_SetTevOp  (GX_TEVSTAGE0, GX_PASSCLR);

    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    lights = 0;
}

void Renderer::SetLightAmbient(U8 r, U8 g, U8 b, U8 a) {
    GX_SetChanAmbColor(GX_COLOR0A0, GXColor{r, g, b, a});
}

void Renderer::SetLightDiffuse(U8 ID, cFVec3& pos, float distattn, float brightness, cUCVec4& color) {
    GXLightObj MyLight;
    guVector lpos = {pos.x, pos.y, pos.z};

    lights |= (1<<ID);

    guVecMultiply(viewMatrix, &lpos, &lpos);
    GX_InitLightPos(&MyLight, lpos.x, lpos.y, lpos.z);
    GX_InitLightColor(&MyLight, (GXColor) { color.x, color.y, color.z, color.w});
    GX_InitLightSpot(&MyLight, 0.0f, GX_SP_OFF);
    GX_InitLightDistAttn(&MyLight, distattn, brightness, GX_DA_MEDIUM); // DistAttn = 20.0  &  Brightness=1.0f (full)
    GX_LoadLightObj(&MyLight, (1<<ID));

    // Turn light ON
    GX_SetNumChans(1);
    GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX, lights, GX_DF_CLAMP, GX_AF_SPOT);
}

void Renderer::PrepareToRenderInVX0(bool pos, bool nrm, bool clr, bool tex, bool clearVtxDesc) {
    if(clearVtxDesc) GX_ClearVtxDesc();
    
    if(pos)     GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    if(nrm)     GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT); 
    if(clr)     GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    if(tex)     GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    if(pos)     GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);     //Positions -> F32
    if(nrm)     GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);     //Normals   -> F32
    if(clr)     GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0); //Color     -> UChar
    if(tex)     GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);     //Textures  -> F32
    
    tex ? GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE) : GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}


void Renderer::PrepareToRenderInVX2(bool pos, bool nrm, bool clr, bool tex) {
    GX_ClearVtxDesc();
    
    if(pos)     GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    if(nrm)     GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT); 
    if(clr)     GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    if(tex)     GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT); 
    
    if(pos)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_U16, 0);     //Positions -> U16
    if(nrm)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);      //Normals   -> S8
    if(clr)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0); //Color     -> UChar

#ifdef OPTIMIZATION_VERTEX_MEMORY
    if(tex)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);      //Textures  -> U8
#else
    if(tex)     GX_SetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);     //Textures  -> U16
#endif

    tex ? GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE) : GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

void Renderer::RenderBegin(U16 VertexCount) {
    nDrawCalls++;
    GX_Begin(GX_QUADS, GX_VTXFMT2, VertexCount);
}

void Renderer::RenderEnd() {
    GX_End();
}

void Renderer::RenderFace(const CubeFace& face, S8 x, S8 y, S8 z) {
    nFacesRendered++;

    auto& UV = tileUVMap[face.tile];
    
    for (int j = 0; j < 4; j++) {
#if OPTIMIZATION_STRUCTS != 0
        GX_Position3u16(positionsCoords[face.direction][0] + cubeFaces[face.direction][j][0] + x,
                        positionsCoords[face.direction][1] + cubeFaces[face.direction][j][1] + y,
                        positionsCoords[face.direction][2] + cubeFaces[face.direction][j][2] + z);
#else
        GX_Position3u16(face.x + cubeFaces[face.direction][j][0] + x,
                        face.y + cubeFaces[face.direction][j][1] + y,
                        face.z + cubeFaces[face.direction][j][2] + z);
#endif

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        GX_Normal3s8(cubeNormals[face.direction][0],
                     cubeNormals[face.direction][1],
                     cubeNormals[face.direction][2]);
#endif

#ifdef OPTIMIZATION_VERTEX_MEMORY
        //GX_Color4u8(255, 255, 255, 255);
        u8 resultInX = UV[0] + static_cast<u8>(tileTexCoords[j][0]);
        u8 resultInY = UV[1] + static_cast<u8>(tileTexCoords[j][1]);
        GX_TexCoord2u8(resultInX, resultInY);
#else
        GX_Color4u8(255, 255, 255, 255);
        GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#endif
// #ifdef OPTIMIZATION_MAPS
//         GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
// #else
//         GX_TexCoord2u16(UV.x + tileTexCoords[j][0], UV.y + tileTexCoords[j][1]);
// #endif
    }
}

void Renderer::RenderFaceIndexed(const CubeFace& face, S8 x, S8 y, S8 z) {
    nFacesRendered++;

    for (U8 j = 0; j < 4; j++) {
#if OPTIMIZATION_STRUCTS != 0
        GX_Position3u16(positionsCoords[face.direction][0] + cubeFaces[face.direction][j][0] + x,
                        positionsCoords[face.direction][1] + cubeFaces[face.direction][j][1] + y,
                        positionsCoords[face.direction][2] + cubeFaces[face.direction][j][2] + z);
#else
        GX_Position3u16(face.x + cubeFaces[face.direction][j][0] + x,
                        face.y + cubeFaces[face.direction][j][1] + y,
                        face.z + cubeFaces[face.direction][j][2] + z);
#endif

#ifndef OPTIMIZATION_NO_LIGHTNING_DATA
        GX_Normal3s8(cubeNormals[face.direction][0],
                     cubeNormals[face.direction][1],
                     cubeNormals[face.direction][2]);
#endif
        
#ifdef OPTIMIZATION_VERTEX_MEMORY
        //GX_Color4u8(255, 255, 255, 255);
#else
        GX_Color4u8(255, 255, 255, 255);
#endif
        
        GX_TexCoord1x8(j);
    }
}

void Renderer::RenderCube(const Cubito& cube, cFVec3& worldPos, cFVec3& angle) {
    cFVec3 position = cFVec3(static_cast<float>(cube.x) + worldPos.x,
                             static_cast<float>(cube.y)  + worldPos.y,
                             static_cast<float>(cube.z)  + worldPos.z);
    ObjectView(position.x, position.y, position.z,
                      angle.x, angle.y, angle.z,
                      1.0f, 1.0f, 1.0f);


    if(hasProperty(cube.type, FOLIAGE)) {
        RenderBegin(2 * 4);
        for (auto& currentFace : cube.face) {
            if(currentFace.tile != NUM_TILES)   RenderFace(currentFace);
        }
        RenderEnd();
    }else {
        RenderBegin(24);
        for (auto& currentFace : cube.face) {
            RenderFace(currentFace);
        }
        RenderEnd();
    }
}

void Renderer::RenderCubeVector(const Vector<Cubito>& cubes, U16 validBlocks) {
    RenderBegin(validBlocks);
    for(auto& cubito: cubes) {
        if(!cubito.visible) continue;
        for (auto& currentFace : cubito.face) {
            if(currentFace.tile != NUM_TILES) {
                RenderFace(currentFace, cubito.x, cubito.y, cubito.z);
            }
        }
    }
    RenderEnd();
}

void Renderer::RenderCubeVectorIndexed(const Vector<Cubito>& cubes, U16 validBlocks) {
    RenderBegin(validBlocks);
    for(auto& cubito: cubes) {
        if(!cubito.visible) continue;
        for (auto& currentFace : cubito.face) {
            if(currentFace.tile != NUM_TILES) {
                RenderFaceIndexed(currentFace, cubito.x, cubito.y, cubito.z);
            }
        }
    }
    RenderEnd();
}

void Renderer::RenderFaceVector(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks) {
    RenderBegin(validBlocks);
    for (auto& currentFaceVec : faces) {
        auto& currentFace = currentFaceVec.first;
        auto& cubito = currentFaceVec.second;
        RenderFace(currentFace, cubito.x, cubito.y, cubito.z);
    }
    RenderEnd();
}

void Renderer::RenderFaceVectorIndexed(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks) {
    RenderBegin(validBlocks);
    for (auto& currentFaceVec : faces) {
        auto& currentFace = currentFaceVec.first;
        auto& cubito = currentFaceVec.second;
        RenderFaceIndexed(currentFace, cubito.x, cubito.y, cubito.z);
    }
    RenderEnd();
}

void Renderer::RenderBoundingBox(S16 originX, S16 originY, S16 originZ, U16 sizeX, U16 sizeY, cUCVec3& color, bool RenderCross) {
    // Set the view for the cube based on origin and size
    //GRRLIB_ObjectView(originX, originY, originZ, 0, 0, 0.0f, size, size, size);
    ObjectView(originX, originY, originZ, 0, 0, 0.0f, sizeX, sizeY, sizeX);

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

void Renderer::RenderSphere(f32 r, int lats, int longs, bool filled, u32 col) {
    const f32 dtheta = 2 * M_PI / longs;

    for(int i = 0; i <= lats; i++) {
        const f32 lat0 = M_PI * (-0.5f + (f32) (i - 1) / lats);
        const f32 z0  = sinf(lat0);
        const f32 zr0 = cosf(lat0);

        const f32 lat1 = M_PI * (-0.5f + (f32) i / lats);
        const f32 z1 = sinf(lat1);
        const f32 zr1 = cosf(lat1);

        GX_Begin((filled == true) ? GX_TRIANGLESTRIP : GX_LINESTRIP,
             GX_VTXFMT0, 2 * (longs + 1));

        for(int j = 0; j <= longs; j++) {
            const f32 lng = dtheta * (f32) (j - 1);
            const f32 x = cosf(lng);
            const f32 y = sinf(lng);

            GX_Position3f32(x * zr0 * r, y * zr0 * r, z0 * r);
            GX_Normal3f32(x * zr0 * r, y * zr0 * r, z0 * r);
            GX_Color1u32(col);
            GX_Position3f32(x * zr1 * r, y * zr1 * r, z1 * r);
            GX_Normal3f32(x * zr1 * r, y * zr1 * r, z1 * r);
            GX_Color1u32(col);
        }
        
        GX_End();
    }
}

void Renderer::CallDisplayList(void* list, U32 size) {
    nDrawCalls++;
    GX_CallDispList(list, size);
}

//Start from the first GX command after VSync, and end after GX_DrawDone().
// GX_SetDrawDone();
void Renderer::RenderGX(bool VSYNC) {
    //GX_DrawDone();         
    //GX_InvalidateTexAll();

    //currentFrameBuffer ^= 1;  // Toggle framebuffer index
    
    SetDepth(GX_TRUE, DEPTH_MODE::LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    
    GX_CopyDisp      (frameBuffers[currentFrameBuffer], GX_TRUE);
    
    GX_SetDrawDone();

    if(VSYNC) VIDEO_WaitVSync();
    GX_WaitDrawDone();

    
    
    // GX_SetDrawDone();
    //
    // if(!VSYNC) {
    //     GX_WaitDrawDone();
    //     VIDEO_SetNextFramebuffer(frameBuffers[currentFrameBuffer]);
    //     VIDEO_Flush();
    // }
    
    // VIDEO_SetNextFramebuffer(frameBuffers[currentFrameBuffer]);  
    // VIDEO_Flush();                      // Flush video buffer to screen
    // VIDEO_WaitVSync();                  // Wait for screen to update
    //
    // // Interlaced screens require two frames to update
    // if (videoMode->viTVMode &VI_NON_INTERLACE) {
    //     VIDEO_WaitVSync();
    // }
}

u16 tileTexCoords[4][2] = {
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1}
};


U8 positionsCoords[6][3] = {
    {1, 0, 0},
    {0, 0, 0},
    {0, 1, 0},
    {0, 0, 0},
    {0, 0, 1},
    {0, 0, 0}
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