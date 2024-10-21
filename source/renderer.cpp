#include <common.h>
#include <ogc/gx.h>
#include <renderer.h>
#include <utilities.h>

#include <grrlib.h>


using namespace poyo;

extern U16 vertices[8][3];
extern U16 edges[12][2];
extern U16 diagonals[12][2];
extern u16 tileTexCoords[4][2];

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

void Renderer::SetTextureCoordScaling(U8 unit, U16 scaleX, U16 scaleY) {
    GX_SetTexCoordScaleManually(unit, GX_TRUE, scaleX, scaleY); //GX_TEXCOORD0
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
}

void Renderer::RenderCube(const Cubito& cube, cFVec3& worldPos, cFVec3& angle) {
    cFVec3 position = cFVec3(static_cast<float>(cube.x) + worldPos.x,
                             static_cast<float>(cube.y)  + worldPos.y,
                             static_cast<float>(cube.z)  + worldPos.z);
    GRRLIB_ObjectView(position.x, position.y, position.z,
                      angle.x, angle.y, angle.z,
                      1.0f, 1.0f, 1.0f);

    GX_Begin(GX_QUADS, GX_VTXFMT2, 24);
    for (auto& currentFace : cube.face) {
        for (int j = 0; j < 4; j++) {

            GX_Position3u16(currentFace.x + cubeFaces[currentFace.direction][j][0],
                            currentFace.y + cubeFaces[currentFace.direction][j][1],
                            currentFace.z + cubeFaces[currentFace.direction][j][2]);
            //GX_Color1u32 old
            //GX_Color1x8(255);
            GX_Normal3s8(cubeNormals[currentFace.direction][0], cubeNormals[currentFace.direction][1], cubeNormals[currentFace.direction][2]);
            //GX_Normal3f32(cubeNormals[currentFace.direction][j], cubeNormals[currentFace.direction][j], cubeNormals[currentFace.direction][j]);
            //GX_Color1u32(0xFFFFFFFF);
            GX_Color4u8(255, 255, 255, 255);
            //GX_TexCoord2f32 old

            auto& UV = tileUVMap[currentFace.tile];

#ifdef OPTIMIZATION_MAPS
            GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#else
            GX_TexCoord2u16(UV.x + tileTexCoords[j][0], UV.y + tileTexCoords[j][1]);
#endif
        }
    }
    GX_End();
}

void Renderer::RenderCubeVector(const Vector<Cubito>& cubes, U16 validBlocks, cFVec3& worldPos) {
    GRRLIB_ObjectView(worldPos.x, worldPos.y, worldPos.z,
                      0.0f, 0.0f, 0.0f,
                      1.0f, 1.0f, 1.0f);
    
    GX_Begin(GX_QUADS, GX_VTXFMT2, validBlocks);
    for(auto& cubito: cubes) {
        if(cubito.type == BLOCK_AIR) continue; //TODO: FIX THIS WHEN CHANGING CUBE
        for (auto& currentFace : cubito.face) {
            for (int j = 0; j < 4; j++) {
                GX_Position3u16(currentFace.x + cubeFaces[currentFace.direction][j][0] + cubito.x,
                                currentFace.y + cubeFaces[currentFace.direction][j][1] + cubito.y,
                                currentFace.z + cubeFaces[currentFace.direction][j][2] + cubito.z);
                GX_Normal3s8(cubeNormals[currentFace.direction][0], cubeNormals[currentFace.direction][1], cubeNormals[currentFace.direction][2]);
                GX_Color4u8(255, 255, 255, 255);
                auto& UV = tileUVMap[currentFace.tile];

#ifdef OPTIMIZATION_MAPS
                GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#else
                GX_TexCoord2u16(UV.x + tileTexCoords[j][0], UV.y + tileTexCoords[j][1]);
#endif
            }
        }
    }

    GX_End();
}

void Renderer::RenderCubeVector2(const Vector<Cubito>& cubes, U16 validBlocks) {
    GX_Begin(GX_QUADS, GX_VTXFMT2, validBlocks);
    for(auto& cubito: cubes) {
        if(cubito.type == BLOCK_AIR) continue; //TODO: FIX THIS WHEN CHANGING CUBE
        for (auto& currentFace : cubito.face) {
            for (int j = 0; j < 4; j++) {
                GX_Position3u16(currentFace.x + cubeFaces[currentFace.direction][j][0] + cubito.x,
                                currentFace.y + cubeFaces[currentFace.direction][j][1] + cubito.y,
                                currentFace.z + cubeFaces[currentFace.direction][j][2] + cubito.z);
                GX_Normal3s8(cubeNormals[currentFace.direction][0], cubeNormals[currentFace.direction][1], cubeNormals[currentFace.direction][2]);
                GX_Color4u8(255, 255, 255, 255);
                auto& UV = tileUVMap[currentFace.tile];

#ifdef OPTIMIZATION_MAPS
                GX_TexCoord2u16(UV[0] + tileTexCoords[j][0], UV[1] + tileTexCoords[j][1]);
#else
                GX_TexCoord2u16(UV.x + tileTexCoords[j][0], UV.y + tileTexCoords[j][1]);
#endif
            }
        }
    }

    GX_End();
}

void Renderer::RenderBoundingBox(S16 originX, S16 originY, S16 originZ, U16 size, cUCVec3& color, bool RenderCross) {
    // Set the view for the cube based on origin and size
    GRRLIB_ObjectView(originX, originY, originZ, 0, 0, 0.0f, size, size, size);

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