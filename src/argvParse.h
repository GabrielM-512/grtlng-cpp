#pragma once

namespace argvParse {
    struct ProgramArgs {
        bool interpret = false;
        char *filePath = nullptr;
    };

    ProgramArgs parse(int argc, char* argv[]);
}