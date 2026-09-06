cmake_minimum_required(VERSION 4.3)
project(grtlng_cpp)

set(CMAKE_CXX_STANDARD 26)

add_executable(grtlng_cpp
        src/main.cpp
        src/global.h
        src/util/fileIO.cpp
        src/util/fileIO.h
        src/compiler/lexing.cpp
        src/compiler/lexing.h
        src/compiler/lexing.h
)

target_compile_options(grtlng_cpp
        PUBLIC -Wall
        PUBLIC -Wextra
        PUBLIC -Werror
        PUBLIC -Ofast
)
