#pragma once
#include "../../compiler/compiler.h"
#include "interpreting.h"

namespace BytecodeCompilation {

     class CompileError : std::runtime_error {
     public:
          std::string message;

          explicit CompileError(const std::string& message) : runtime_error(message), message(message) {}
     };

     Bytecode::Program compile(const Compiler::CompileResult& ast);
}