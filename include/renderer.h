#ifndef INCLUDE_RENDERER_H_
#define INCLUDE_RENDERER_H_ 1

namespace poyo {
    enum class BLEND_MODE;
    class Renderer {
     public:
        Renderer() = delete;
        ~Renderer() = delete;

        static void Initialize();
        
        static void SetTextureCoordScaling(U8 unit, U16 scaleX, U16 scaleY);

        static void EnableBlend(BLEND_MODE mode);
        static void DisableBlend();

        static void BindTexture(GXTexObj& obj, U8 unit);

        static void ResetDrawCalls();
        static U32 DrawCalls();
        static U32 FacesDrawn();

        static void ObjectView(f32 posx, f32 posy, f32 posz,
                               f32 angx = 0, f32 angy = 0, f32 angz = 0,
                               f32 scalx = 1, f32 scaly = 1, f32 scalz = 1);
        
        static void PrepareToRender(bool pos, bool nrm, bool clr, bool tex);

        static void RenderBegin(U16 VertexCount);
        static void RenderEnd();

        static void RenderFace(const CubeFace& face);                   // No Batching
        static void RenderFace(const CubeFace& face, S8 x, S8 y, S8 z); // Batching
        static void RenderCube(const Cubito& cube, cFVec3& worldPos = cFVec3(0), cFVec3& angle = cFVec3(0)); // No Batching
        static void RenderCubeVector(const Vector<Cubito>& cubes, U16 validBlocks);                 //Only for Batching
        static void RenderFaceVector(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks); //Only for Batching
        static void RenderBoundingBox(S16 originX, S16 originY, S16 originZ, U16 size, cUCVec3& color, bool RenderCross = false);

        static void CallDisplayList(void* list, U32 size);
    };

    enum class BLEND_MODE {
        ALPHA  = 0,    /**< Alpha Blending. */
        ADD    = 1,    /**< Additive Blending. */
        SCREEN = 2,    /**< Alpha Light Blending. */
        MULTI  = 3,    /**< Multiply Blending. */
        INV    = 4,    /**< Invert Color Blending. */ 
    };
}

#endif