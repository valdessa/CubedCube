#include <common.h>
#include <utilities.h>

using namespace poyo;

void poyo::mapTileUVs(U8 tilesetWidth) {
    for (U8 tile = 0; tile < NUM_TILES; tile++) {
        U16 U = tile % tilesetWidth;     //  U (X)
        U16 V = tile / tilesetWidth;     //  V (Y)
#ifdef OPTIMIZATION_MAPS
        tileUVMap[tile][0] = U;
        tileUVMap[tile][1] = V;
#else
        tileUVMap[tile] = USVec2{U, V};
#endif
    }
}

double poyo::convertBytesToKilobytes(const size_t bytes) {
    return static_cast<double>(bytes) / 1024.0; // 1 KB = 1024 bytes
}

String poyo::formatThousands(size_t value) {
    String num_str = std::to_string(value);  // Convert the number to a string
    auto insert_position = num_str.length() - 3;   // Start 3 digits from the end

    // Insert commas every three digits
    while (insert_position > 0) {
        num_str.insert(insert_position, ".");
        insert_position -= 3;
    }

    return num_str;
}
