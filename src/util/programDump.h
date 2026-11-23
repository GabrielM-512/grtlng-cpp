#pragma once
#include "../interpreter/interpreter.h"

namespace programDump {
    void dumpBytecode(const Bytecode::Program& program, const char* filepath);
    Bytecode::Program readBytecodeFile(const char *filepath);

    class LoadError : public std::runtime_error {
    public:
        LoadError(const std::string& message) : runtime_error(message) {}
    };
}
