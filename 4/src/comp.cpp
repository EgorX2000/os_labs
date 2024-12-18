#include <cmath>
#include <iostream>

extern "C" float SinIntegral(float A, float B, float e);
extern "C" int* Sort(int* array, int size);

int main() {
    int command;
    while (true) {
        std::cout << "Enter command: ";
        std::cin >> command;

        if (command == 1) {
            float A, B, e;
            std::cin >> A >> B >> e;
            std::cout << "Result: " << SinIntegral(A, B, e) << std::endl;
            break;
        } else if (command == 2) {
            int size;
            std::cin >> size;
            int* array = new int[size];
            for (int i = 0; i < size; ++i) {
                std::cin >> array[i];
            }
            Sort(array, size);
            std::cout << "Sorted array: ";
            for (int i = 0; i < size; ++i) {
                std::cout << array[i] << " ";
            }
            std::cout << std::endl;
            delete[] array;
            break;
        } else {
            std::cout << "Invalid command. Use 0 to switch library, 1 or 2 to "
                         "execute functions."
                      << std::endl;
        }
    }

    return 0;
}
