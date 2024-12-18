#include <windows.h>

#include <iostream>
#include <string>

typedef float (*SinIntegralFunc)(float, float, float);
typedef int* (*SortFunc)(int*, int);

void RunProgram(HINSTANCE& lib) {
    SinIntegralFunc SinIntegral =
        (SinIntegralFunc)GetProcAddress(lib, "SinIntegral");
    SortFunc Sort = (SortFunc)GetProcAddress(lib, "Sort");

    if (!SinIntegral || !Sort) {
        std::cerr << "Error loading functions from the library." << std::endl;
        return;
    }

    int command;
    while (true) {
        std::cout << "Enter command: ";
        std::cin >> command;

        if (command == 0) {
            std::cout << "Switching library. Enter 1 for Lib1 or 2 for Lib2: ";
            int libChoice;
            std::cin >> libChoice;

            FreeLibrary(lib);
            if (libChoice == 1) {
                lib = LoadLibrary("libcontract1.dll");
            } else if (libChoice == 2) {
                lib = LoadLibrary("libcontract2.dll");
            } else {
                std::cerr << "Invalid library choice!" << std::endl;
                continue;
            }

            if (!lib) {
                std::cerr << "Error loading library." << std::endl;
                exit(-1);
            }

            SinIntegral = (SinIntegralFunc)GetProcAddress(lib, "SinIntegral");
            Sort = (SortFunc)GetProcAddress(lib, "Sort");

            if (!SinIntegral || !Sort) {
                std::cerr << "Error loading functions from the new library."
                          << std::endl;
                exit(-1);
            }

            std::cout << "Library switched successfully." << std::endl;
            continue;
        } else if (command == 1) {
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
}

int main() {
    HINSTANCE lib = LoadLibrary("libcontract1.dll");

    if (!lib) {
        std::cerr << "Error loading library." << std::endl;
        return -1;
    }

    RunProgram(lib);

    FreeLibrary(lib);
    return 0;
}
