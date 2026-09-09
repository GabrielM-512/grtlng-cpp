#pragma once
#include <map>

#include "compiler/lexing.h"
#include "interpreter/runtimeException.h"

namespace Error {

    struct CompileError {
        std::string message;
        std::string hint;
        Lexing::Tokens::Token& token;
    };

    class ErrorHandler {
        std::string& source;
        std::map<int, CompileError> compileErrors;
        void printErrorLine(Lexing::Tokens::Token token) const;
    public:
        ErrorHandler(std::string& source) : source(source) {}

        void compileError();

        void runtimeError(Interpreting::RuntimeException& error);
    };
}
