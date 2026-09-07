#include "compiler.h"

#include <iostream>

#include "lexing.h"
#include "../AST/expr.h"
#include "parsing/parsing.h"

Compiler::CompileResult Compiler::compile(const std::string& source) {
    CompileResult result = {nullptr};

    std::vector<Lexing::Tokens::Token> tokens = Lexing::scan(source);

    Parsing::Parser parser(tokens);
    result.expr = parser.parse();

    return result;
}
