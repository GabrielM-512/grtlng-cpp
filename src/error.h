#pragma once

#include "compiler/lexing.h"
#include "interpreter/runtimeException.h"

namespace Error {

    struct CompileError {
        std::string message;
        std::string hint;
        Lexing::Token token;
    };

    class ErrorHandler {
        std::string& source;
        std::vector<CompileError> compileErrors;
        void printErrorLine(Lexing::Token token) const;
    public:
        ErrorHandler(std::string& source) : source(source) {}

        void compileError(std::string message, std::string hint, Lexing::Token token);
        void printErrors();

        void runtimeError(Interpreting::RuntimeException& error);
    };
}
