#pragma once
#include "../interpreter/bytecode/interpreting.h"

namespace programDump {
    void dumpBytecode(const Bytecode::Program& program, const char* filepath);
    Bytecode::Program readBytecode(const char *filepath);

    class LoadError : public std::runtime_error {
    public:
        LoadError(const std::string& message) : runtime_error(message) {}
    };
}
