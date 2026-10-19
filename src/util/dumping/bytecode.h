#pragma once
#include "../../interpreter/bytecode/interpreting.h"

namespace Bytecode {
    void dump(const Program& program, const char* filepath);
    Program load(const char *filepath);
}
