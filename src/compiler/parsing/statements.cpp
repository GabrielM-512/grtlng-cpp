#include "parsing.h"

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