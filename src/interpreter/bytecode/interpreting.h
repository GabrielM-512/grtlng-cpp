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
    };

    struct Program {
        std::vector<u8> code;
        std::vector<u16> lines;
        std::vector<Value::Value> constants;
        u16 globalCount {};
    };

    i32 interpret(const Compiler::CompileResult &ast, bool decompile);
}