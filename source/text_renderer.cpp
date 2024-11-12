#include <typedefs.h>
#include <gccore.h>

#include <text_renderer.h>

#include <cstring>

using namespace poyo;

//Font TPL data
#include "font_tpl.h"
#include "font.h"

#define TEXT_GLYPH_WIDTH 64  //8
#define TEXT_GLYPH_HEIGHT 64 //16

static void mapTileUVs(u8 tilesetWidth, HashMap<u8, Pair<u16, u16>>& tileUVMap) {
    for (u8 tile = 0; tile < 96; tile++) {
        auto U = tile % tilesetWidth;     // Coordenada U (X)
        auto V = tile / tilesetWidth;     // Coordenada V (Y)
        tileUVMap[tile] = {U, V};
    }
}

TextRenderer::TextRenderer() : size_{8, 16} {
    mapTileUVs(8, tileUVMap_);
    TPL_OpenTPLFromMemory(&fontTPL, (void *)font_tpl, font_tpl_size);
    TPL_GetTexture(&fontTPL, fontTextureId, &fontTexture);
    GX_InitTexObjFilterMode(&fontTexture, GX_NEAR, GX_NEAR);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GX_InvalidateTexAll();
}

TextRenderer::~TextRenderer() {
    
}

void TextRenderer::beginRender() {
    GX_LoadTexObj(&fontTexture, GX_TEXMAP0);
    GX_SetNumTevStages(1);
    GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
    GX_SetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, TEXT_GLYPH_WIDTH, TEXT_GLYPH_HEIGHT);
    
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XY, GX_U16, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);
}

void TextRenderer::setFontSize(cUSVec2& size) {
    size_ = size;
}

void TextRenderer::render(USVec2 pos, const char* text) {
    u16 len = static_cast<u16>(strlen(text));

    const u16 tileTexCoords[4][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    };
    
    GX_Begin(GX_QUADS, GX_VTXFMT1, len * 4);
    for (u16 i = 0; i < len; i++) {
        u16 glyph = text[i] - ' ';
        
        // GX_Position2u16(pos.x, pos.y);
        // GX_TexCoord2u16(glyph, 0);
        // GX_Position2u16(pos.x + size_.x, pos.y);
        // GX_TexCoord2u16(glyph + 1, 0);
        // GX_Position2u16(pos.x + size_.x, pos.y + size_.y);
        // GX_TexCoord2u16(glyph + 1, 1);
        // GX_Position2u16(pos.x, pos.y + size_.y);
        // GX_TexCoord2u16(glyph, 1);

        const auto& UV = tileUVMap_[glyph];

        GX_Position2u16(pos.x, pos.y);
        GX_TexCoord2u16(UV.first + tileTexCoords[0][0], UV.second + tileTexCoords[0][1]);
        
        GX_Position2u16(pos.x + size_.x, pos.y);
        GX_TexCoord2u16(UV.first + tileTexCoords[1][0], UV.second + tileTexCoords[1][1]);
        
        GX_Position2u16(pos.x + size_.x, pos.y + size_.y);
        GX_TexCoord2u16(UV.first + tileTexCoords[2][0], UV.second + tileTexCoords[2][1]);
        
        GX_Position2u16(pos.x, pos.y + size_.y);
        GX_TexCoord2u16(UV.first + tileTexCoords[3][0], UV.second + tileTexCoords[3][1]);

        
        pos.x += size_.x;
    }
    GX_End();
}
