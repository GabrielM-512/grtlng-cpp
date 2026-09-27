#pragma once

namespace argvParse {
    struct ProgramArgs {
        bool interpret = false;
        bool decompile = false;
        char *filePath = nullptr;
    };

    ProgramArgs parse(int argc, char* argv[]);
}