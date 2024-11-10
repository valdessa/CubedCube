//Blocks TPL data

#include "bloquitos_tpl.h"
#include "bloquitos.h"

// Font
#include "Karma_ttf.h"

static TPLFile bloquitosTPL;
static GXTexObj blocksTexture;

void loadResources() {
    TPL_OpenTPLFromMemory(&bloquitosTPL, (void*)bloquitos_tpl, bloquitos_tpl_size);
    TPL_GetTexture(&bloquitosTPL, blocksTextureID, &blocksTexture);
    GX_InitTexObjFilterMode(&blocksTexture, GX_NEAR_MIP_NEAR, GX_NEAR);
    GX_SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);
    GX_InvalidateTexAll();
}