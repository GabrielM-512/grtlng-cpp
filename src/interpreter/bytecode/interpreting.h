#pragma once

#include "../../compiler/compiler.h"

namespace Bytecode {
    enum Operation : u8 {
        RETURN,
        PRINT,

        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,

        NEGATE,
        NOT,

        EQUALS,
        LESS,
        MORE,
        LESS_EQUALS,
        MORE_EQUALS,

        CALL,

        POP,
        POP_N,

        LOAD_I8,
        FALSE,
        TRUE,
        LOAD_CONSTANT,
        LOAD_GLOBAL,
        LOAD_LOCAL,

        SET_GLOBAL,
        SET_LOCAL,

        JUMP,
        JUMP_FALSE,
        JUMP_TRUE,

        LOOP,

        EXIT
    };

    struct Program {
        std::vector<u8> code;
        std::vector<u16> lines;
        std::vector<Value::Value> constants;
        u16 globalCount {};
    };

    i32 interpret(const Compiler::CompileResult &ast, bool decompile);
}