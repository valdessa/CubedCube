#ifndef INCLUDE_NOISE_H_
#define INCLUDE_NOISE_H_ 1

#include <regex>

namespace poyo {
    class Noise {
     public:
        Noise() {
            // Fill the permutation array with values 0-255
            for (int i = 0; i < 256; i++) {
                permutation[i] = i;
            }
            // Shuffle the array
            for (int i = 0; i < 256; i++) {
                int j = rand() % 256;
                std::swap(permutation[i], permutation[j]);
            }
            // Duplicate the array
            for (int i = 0; i < 256; i++) {
                permutation[256 + i] = permutation[i];
            }
        }
        ~Noise() {
            
        }

        float fade(float t) {
            return t * t * t * (t * (t * 6 - 15) + 10); // Fade function as defined by Ken Perlin
        }

        float lerp(float a, float b, float t) {
            return a + t * (b - a); // Linear interpolation
        }

        float grad(int hash, float x, float y) {
            int h = hash & 3; // Convert low 2 bits of hash code
            float u = h < 2 ? x : y; // and a random gradient value
            float v = h < 2 ? y : x;
            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v); // Gradient value 1-3
        }

        float perlin(float x, float y) {
            // Find unit grid cell coordinates
            int X = (int)floor(x) & 255;
            int Y = (int)floor(y) & 255;

            // Relative x and y in the grid cell
            x -= floor(x);
            y -= floor(y);

            // Compute fade curves for x and y
            float u = fade(x);
            float v = fade(y);

            // Hash coordinates of the square corners
            int aa = (permutation[X] + Y) & 255;
            int ab = (permutation[X] + Y + 1) & 255;
            int ba = (permutation[X + 1] + Y) & 255;
            int bb = (permutation[X + 1] + Y + 1) & 255;

            // And add blended results from the four corners of the square
            float res = lerp(
                lerp(grad(permutation[aa], x, y),
                     grad(permutation[ba], x - 1, y),
                     u),
                lerp(grad(permutation[ab], x, y - 1),
                     grad(permutation[bb], x - 1, y - 1),
                     u),
                v);

            return res;
        }

        int permutation[512];
    };
}

#endif