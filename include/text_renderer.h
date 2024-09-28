#ifndef INCLUDE_TEXT_RENDERER_H_
#define INCLUDE_TEXT_RENDERER_H_ 1

namespace poyo {
    class TextRenderer {
     public:
        TextRenderer();
        ~TextRenderer();

        void beginRender();

        void setFontSize(cUSVec2& size);

        void render(USVec2 pos, const char* text);
        
     private:
        USVec2 size_;
        HashMap<u8, Pair<u16, u16>> tileUVMap_;
        TPLFile fontTPL;
        GXTexObj fontTexture;
    };
}

#endif