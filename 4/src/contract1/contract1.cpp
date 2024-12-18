#include <algorithm>
#include <cmath>

extern "C" __declspec(dllexport) float SinIntegral(float A, float B, float e) {
    float result = 0.0f;
    for (float x = A; x < B; x += e) {
        result += std::sin(x) * e;
    }
    return result;
}

extern "C" __declspec(dllexport) int* Sort(int* array, int size) {
    for (int i = 0; i < size - 1; ++i) {
        for (int j = 0; j < size - i - 1; ++j) {
            if (array[j] > array[j + 1]) {
                std::swap(array[j], array[j + 1]);
            }
        }
    }
    return array;
}