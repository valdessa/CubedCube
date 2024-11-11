#include <common.h>
#include <utilities.h>
#include <algorithm>

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

String poyo::formatThousands(size_t numero) {
    std::string resultado;
    bool negativo = false;

    // Si el número es negativo, lo marcamos y lo convertimos en positivo
    if (numero < 0) {
        negativo = true;
        numero = -numero;
    }

    // Convertimos el número a una cadena
    std::string numStr = std::to_string(numero);

    // Añadimos los dígitos de derecha a izquierda, con los separadores de miles
    int count = 0;
    for (int i = numStr.size() - 1; i >= 0; --i) {
        if (count > 0 && count % 3 == 0) {
            resultado += ',';  // Añadimos el separador de miles
        }
        resultado += numStr[i];
        count++;
    }

    // Si el número era negativo, lo añadimos al principio
    if (negativo) {
        resultado += '-';
    }

    // Invertimos la cadena para devolver el número en el orden correcto
    std::reverse(resultado.begin(), resultado.end());
    
    return resultado;
}
