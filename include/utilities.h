#ifndef INCLUDE_UTILITIES_H_
#define INCLUDE_UTILITIES_H_ 1

namespace poyo {
    void mapTileUVs(U8 tilesetWidth);
    double convertBytesToKilobytes(size_t bytes);
    String formatThousands(size_t value);
}

#endif
