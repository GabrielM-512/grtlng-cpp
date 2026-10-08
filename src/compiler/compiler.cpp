#include "compiler.h"

#include <iostream>

#include "lexing.h"
#include "resolving.h"
#include "parsing/parsing.h"

Compiler::CompileResult Compiler::compile(const std::string& source, Error::ErrorHandler& errorHandler) {
    CompileResult result;

    std::vector<Lexing::Token> tokens = Lexing::scan(source);

    Parsing::Parser parser(tokens, errorHandler);
    result.tree = parser.parse();

    result.zeroReturn = new Stmt::Return(Parsing::Parser::fakeToken(), new Expr::Number(Parsing::Parser::fakeToken(), 0));

    if (parser.hadParseError()) result.success = false;

    if (parser.hadFatalParseError()) return result;


    Resolving::resolve(result, errorHandler);

    return result;
}
