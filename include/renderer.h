#ifndef INCLUDE_RENDERER_H_
#define INCLUDE_RENDERER_H_ 1

namespace poyo {
    enum class BLEND_MODE;
    enum class VIDEO_MODE;
    enum class CULL_MODE;
    enum class DEPTH_MODE;
    class Renderer {
     public:
        Renderer() = delete;
        ~Renderer() = delete;

        static int InitializeGX();
        static void Initialize();

        static void Exit();

        static void Set3DMode(const class Camera& cam);
        static void Set2DMode();
        
        static void SetTextureCoordScaling(U8 unit, U16 scaleX, U16 scaleY);
        static void SetBackgroundColour(U8 r, U8 g, U8 b, U8 a);

        static void SetCameraSettings(cFVec3& position, cFVec3& up, cFVec3& look);

        static void SetCullFace(CULL_MODE mode);
        static void SetBlend(BLEND_MODE mode);
        static void SetDepth(bool enable, DEPTH_MODE mode, bool update);

        static void EnableFog();
        static void DisableFog();
        
        static void SetAlphaTest(bool enable);

        static void BindTexture(GXTexObj& obj, U8 unit);

        static void SetLightOff();
        static void SetLightAmbient(U8 r, U8 g, U8 b, U8 a);
        static void SetLightDiffuse(U8 ID, cFVec3& pos, float distattn = 1.0f, float brightness = 1.0f, cUCVec4& color = UCVec4(255));

        static void ResetDrawCalls();
        static U32 DrawCalls();
        static U32 FacesDrawn();
        static bool isAntialiased();
        static int ScreenWidth();
        static int ScreenHeight();
        static VIDEO_MODE VideoMode();
        static const Mtx& ViewMatrix();

        //Todo: make 1 only using one
        static void CalculateModelMatrix(Mtx& modelToFill, const Transform& trans);
        static void CalculateModelMatrix(Mtx& modelToFill, f32 posx, f32 posy, f32 posz);
        static void ObjectView(); //Only Camera View is Applied!!
        static void ObjectView(const Transform& trans);
        static void ObjectView(f32 posx, f32 posy, f32 posz,
                               f32 angx = 0, f32 angy = 0, f32 angz = 0,
                               f32 scalx = 1, f32 scaly = 1, f32 scalz = 1);
        
        static void PrepareToRenderInVX0(bool pos, bool nrm, bool clr, bool tex, bool clearVtxDesc = true);
        static void PrepareToRenderInVX2(bool pos, bool nrm, bool clr, bool tex);

        static void RenderBegin(U16 VertexCount);
        static void RenderEnd();
        
        static void RenderFace(const CubeFace& face, S8 x = 0, S8 y = 0, S8 z = 0);         // No Batching / Batching
        static void RenderFaceIndexed(const CubeFace& face, S8 x = 0, S8 y = 0, S8 z = 0);  // No Batching / Batching
        static void RenderCube(const Cubito& cube, cFVec3& worldPos = cFVec3(0), cFVec3& angle = cFVec3(0)); // No Batching
        
        static void RenderCubeVector(const Vector<Cubito>& cubes, U16 validBlocks);                        //Only for Batching
        static void RenderCubeVectorIndexed(const Vector<Cubito>& cubes, U16 validBlocks);                 //Only for Batching
        static void RenderFaceVector(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks);        //Only for Batching
        static void RenderFaceVectorIndexed(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks); //Only for Batching
        static void RenderBoundingBox(S16 originX, S16 originY, S16 originZ, U16 sizeX, U16 sizeY, cUCVec3& color, bool RenderCross = false);

        static void RenderSphere(f32 r, int lats, int longs, bool filled, u32 col);

        static void CallDisplayList(void* list, U32 size);

        static void RenderGX(bool VSYNC);
    };

    enum class BLEND_MODE {
        ALPHA  = 0,    /**< Alpha Blending. */
        ADD    = 1,    /**< Additive Blending. */
        SCREEN = 2,    /**< Alpha Light Blending. */
        MULTI  = 3,    /**< Multiply Blending. */
        INV    = 4,    /**< Invert Color Blending. */
        MODE_OFF
    };

    enum class VIDEO_MODE {
        INTERLACE     = 0,		/*!< Video mode INTERLACED. */
        NON_INTERLACE = 1,		/*!< Video mode NON INTERLACED */
        PROGRESSIVE   = 2,		/*!< Video mode PROGRESSIVE. Special mode for higher quality */
        VIDEO_ERROR   = 3		
    };

    enum class CULL_MODE {
        MODE_NONE,
        MODE_BACK,
        MODE_FRONT,
        MODE_ALL,
    };

    enum class DEPTH_MODE {
        LEQUAL,
        LESS,
        EQUAL,
    };
    
}

#endif