#pragma once
#include <string>

#include "../error.h"
#include "../AST/stmt.h"

#include "../interpreter/interpreter.h"
#include "../argvParse.h"

namespace Compiler {
    struct SyntaxTree {
        bool success = true;
        std::vector<Stmt::Stmt*> tree;
    };

    class CompileError : std::runtime_error {
    public:
        std::string message;
        Lexing::Token token;
        bool makeError;

        CompileError(const std::string& message, Lexing::Token token, bool makeError) : runtime_error(message), message(message),
                                                                         token(token), makeError(makeError) {}
        CompileError(const std::string& message, Lexing::Token token) : runtime_error(message), message(message),
                                                                         token(token), makeError(false) {}
    };

    Bytecode::Program compile(const argvParse::ProgramArgs& flags);
}
