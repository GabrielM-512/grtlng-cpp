#include "parsing.h"
#include "../compiler.h"

using namespace Parsing;

Stmt::Stmt* Parser::statement() {
    if (match(Lexing::PRINT)) return printStatement();

    return expressionStatement();
}

Stmt::Stmt* Parser::printStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::SEMICOLON);
    return new Stmt::Print(expr);
}

Stmt::Stmt* Parser::expressionStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::SEMICOLON);
    return new Stmt::Expression(expr);
}

Stmt::Stmt* Parser::localDeclarationStatement() {
    Lexing::TokenType dataType = previous.type;
    if (!consume(Lexing::IDENTIFIER, " after data type")) {
        throw Compiler::CompileError("", previous);
    }
    const Lexing::Token name = previous;

    Expr::Expr* value = nullptr;

    if (match(Lexing::EQUALS)) {
        value = expression();
    }

    consume(Lexing::SEMICOLON, " after variable declaration");

    return new Stmt::VariableDeclaration(dataType, name, value);
}