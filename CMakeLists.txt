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
        src/compiler/compiler.cpp
        src/compiler/compiler.h
        src/AST/expr.h
        src/compiler/parsing/parsing.cpp
        src/compiler/parsing/parsing.h
        src/compiler/parsing/expressions.cpp
        src/compiler/parsing/expressions.h
        tool/prettyPrinter.cpp
        tool/prettyPrinter.h
        src/interpreter/interpreting.cpp
        src/interpreter/interpreting.h
        src/value.cpp
        src/value.h
        src/interpreter/environment.cpp
        src/interpreter/environment.h
)

target_compile_options(grtlng_cpp
        PUBLIC -Wall
        PUBLIC -Wextra
        PUBLIC -Werror
        PUBLIC -Ofast
)


include(CTest)

enable_testing()

add_test(NAME End-To-End COMMAND python ../testing/end-to-end/test.py ../testing/end-to-end/config.json multi)

add_test(lexer_partial ${CMAKE_CTEST_COMMAND}
        --build-and-test "${CMAKE_SOURCE_DIR}/testing/partial/lexer"
        "${CMAKE_BINARY_DIR}/testing/partial/lexer"
        --build-generator ${CMAKE_GENERATOR}
        --build-makeprogram ${CMAKE_MAKE_PROGRAM}
        --build-project lexer_test
        --test-command lexer_test)

add_test(parser_partial ${CMAKE_CTEST_COMMAND}
        --build-and-test "${CMAKE_SOURCE_DIR}/testing/partial/parser"
        "${CMAKE_BINARY_DIR}/testing/partial/parser"
        --build-generator ${CMAKE_GENERATOR}
        --build-makeprogram ${CMAKE_MAKE_PROGRAM}
        --build-project parser_partial
        --test-command parser_partial)