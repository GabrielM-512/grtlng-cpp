#include "compiler.h"

#include <iostream>

#include "lexing.h"
#include "../AST/expr.h"
#include "parsing/parsing.h"

Compiler::CompileResult Compiler::compile(const std::string& source, Error::ErrorHandler& errorHandler) {
    CompileResult result = {.success = true, .expr = nullptr};

    std::vector<Lexing::Tokens::Token> tokens = Lexing::scan(source);

    Parsing::Parser parser(tokens, errorHandler);
    result.expr = parser.parse();

    if (parser.hadParseError()) result.success = false;

    return result;
}
