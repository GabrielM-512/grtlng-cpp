cmake_minimum_required(VERSION 4.3)
project(parser_partial)

set(CMAKE_CXX_STANDARD 26)

message(WORKING_DIRECTORY)

add_executable(parser_partial
        /home/gabriel/CLionProjects/grtlng-cpp/testing/partial/parser/parser.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/lexing.h
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/lexing.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/parsing/parsing.h
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/parsing/parsing.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/parsing/expressions.h
        /home/gabriel/CLionProjects/grtlng-cpp/src/compiler/parsing/expressions.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/src/util/fileIO.h
        /home/gabriel/CLionProjects/grtlng-cpp/src/util/fileIO.cpp
        /home/gabriel/CLionProjects/grtlng-cpp/tool/prettyPrinter.h
        /home/gabriel/CLionProjects/grtlng-cpp/tool/prettyPrinter.cpp
)

target_compile_options(parser_partial
        PUBLIC -Wall
        PUBLIC -Wextra
        PUBLIC -Werror
)