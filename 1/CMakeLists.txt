cmake_minimum_required(VERSION 3.10)

project("lab_1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main ParentProcess.cpp)
add_executable(child ChildProcess.cpp)

file(COPY ${CMAKE_SOURCE_DIR}/file.txt DESTINATION ${CMAKE_BINARY_DIR})