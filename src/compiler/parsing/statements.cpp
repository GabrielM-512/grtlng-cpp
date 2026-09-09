#include "parsing.h"
#include "../compiler.h"

using namespace Parsing;

Stmt::Stmt* Parser::statement() {
    if (match(Lexing::Tokens::PRINT)) return printStatement();

    return expressionStatement();
}

Stmt::Stmt* Parser::printStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::Tokens::SEMICOLON);
    return new Stmt::Print(expr);
}

Stmt::Stmt* Parser::expressionStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::Tokens::SEMICOLON);
    return new Stmt::Expression(expr);
}

Stmt::Stmt* Parser::localDeclarationStatement() {
    Lexing::Tokens::TokenType dataType = previous.type;
    if (!consume(Lexing::Tokens::IDENTIFIER, " after data type")) {
        throw Compiler::CompileError("", previous);
    }
    const Lexing::Tokens::Token name = previous;

    Expr::Expr* value = nullptr;

    if (match(Lexing::Tokens::EQUALS)) {
        value = expression();
    }

    consume(Lexing::Tokens::SEMICOLON, " after variable declaration");

    return new Stmt::VariableDeclaration(dataType, name, value);
}