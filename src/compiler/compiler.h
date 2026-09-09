#pragma once
#include <string>

#include "../error.h"
#include "../AST/stmt.h"

namespace Compiler {
    struct CompileResult {
        bool success = true;
        std::vector<Stmt::Stmt*> tree;
    };

    class CompileError : std::runtime_error {
    public:
        std::string message;
        Lexing::Tokens::Token token;
        bool makeError;

        CompileError(const std::string& message, Lexing::Tokens::Token token, bool makeError) : runtime_error(message), message(message),
                                                                         token(token), makeError(makeError) {}
        CompileError(const std::string& message, Lexing::Tokens::Token token) : runtime_error(message), message(message),
                                                                         token(token), makeError(false) {}
    };

    CompileResult compile(const std::string &source, Error::ErrorHandler& errorHandler);
}
