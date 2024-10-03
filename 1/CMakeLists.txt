cmake_minimum_required(VERSION 3.10)

project("lab_1")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/file.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(main ParentProcess.cpp)
add_executable(child ChildProcess.cpp)