#include <algorithm>
#include <cmath>

extern "C" __declspec(dllexport) float SinIntegral(float A, float B, float e) {
    float result = 0.0f;
    for (float x = A; x < B; x += e) {
        result += (std::sin(x) + std::sin(x + e)) * e / 2.0f;
    }
    return result;
}

extern "C" __declspec(dllexport) int* Sort(int* array, int size) {
    std::sort(array, array + size);
    return array;
}