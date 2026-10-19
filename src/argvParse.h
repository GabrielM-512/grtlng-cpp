#pragma once

namespace argvParse {

    enum CompileType : unsigned char {
        COMP_NONE,
        INTERPRET
    };

    enum DumpType : unsigned char {
        DUMP_NONE,
        LOAD,
        DUMP
    };

    struct ProgramArgs {
        CompileType type = INTERPRET;
        DumpType dump = DUMP_NONE;
        bool decompile = false;
        bool printAst = false;
        char *filePath = nullptr;
        char *dumpPath = nullptr;
    };

    ProgramArgs parse(int argc, char* argv[]);
}