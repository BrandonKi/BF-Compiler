cmake_minimum_required(VERSION 3.0.0)
project(BF-Compiler VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 20)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BFCompiler.cpp
)

add_executable(bfc ${SRC})

target_include_directories(bfc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)