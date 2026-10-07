#pragma once

namespace argvParse {

    enum CompileType : unsigned char {
        NONE,
        INTERPRET
    };

    struct ProgramArgs {
        CompileType type = NONE;
        bool decompile = false;
        bool printAst = false;
        char *filePath = nullptr;
    };

    ProgramArgs parse(int argc, char* argv[]);
}