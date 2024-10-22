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
        static U32 FacesDrawn();
        
        static void PrepareToRender(bool pos, bool nrm, bool clr, bool tex);

        static void RenderFace(const CubeFace& face);
        static void RenderCube(const Cubito& cube, cFVec3& worldPos = cFVec3(0), cFVec3& angle = cFVec3(0));
        static void RenderCubeVector(const Vector<Cubito>& cubes, U16 validBlocks, cFVec3& worldPos = cFVec3(0));
        static void RenderCubeVector2(const Vector<Cubito>& cubes, U16 validBlocks);
        static void RenderFaceVector(const Vector<Pair<CubeFace, USVec3>>& faces, U16 validBlocks);
        static void RenderBoundingBox(S16 originX, S16 originY, S16 originZ, U16 size, cUCVec3& color, bool RenderCross = false);
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