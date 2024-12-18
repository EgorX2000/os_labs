cmake_minimum_required(VERSION 3.10)

project("lab_4")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES_LIBS
    contract1/contract1.hpp
    contract2/contract2.hpp
)

add_library(contract1 SHARED contract1/contract1.cpp)
add_library(contract2 SHARED contract2/contract2.cpp)

add_executable(comp comp.cpp)

target_link_libraries(comp
    contract1
)

add_executable(exec exec.cpp)

if(WIN32)
    target_link_libraries(exec kernel32)
endif()
