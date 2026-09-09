#include "compiler.h"

#include <iostream>

#include "lexing.h"
#include "parsing/parsing.h"

Compiler::CompileResult Compiler::compile(const std::string& source, Error::ErrorHandler& errorHandler) {
    CompileResult result;

    std::vector<Lexing::Tokens::Token> tokens = Lexing::scan(source);

    Parsing::Parser parser(tokens, errorHandler);
    result.tree = parser.parse();

    if (parser.hadParseError()) result.success = false;

    return result;
}
