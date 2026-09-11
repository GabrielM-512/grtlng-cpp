#include "parsing.h"
#include "../compiler.h"

using namespace Parsing;

Stmt::Stmt* Parser::statement() {
    if (match(Lexing::PRINT)) return printStatement();
    if (match(Lexing::IF)) return ifStatement();
    if (match(Lexing::WHILE)) return whileStatement();
    if (match(Lexing::FOR)) return forStatement();
    if (match(Lexing::LEFT_BRACE)) return blockStatement();

    return expressionStatement();
}

Stmt::Stmt* Parser::declaration() {
    if (checkTypeIdent()) return localDeclarationStatement();
    return statement();
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
    consume(Lexing::IDENTIFIER, " after data type");

    const Lexing::Token name = previous;

    Expr::Expr* value = nullptr;

    if (match(Lexing::EQUALS)) {
        value = expression();
    }

    consume(Lexing::SEMICOLON, " after variable declaration");

    return new Stmt::VariableDeclaration(dataType, name, value);
}

Stmt::Stmt* Parser::ifStatement() {
    consume(Lexing::LEFT_PAREN, " after \"if\"");
    Expr::Expr* condition = expression();
    consume(Lexing::RIGHT_PAREN, " after if condition");

    Stmt::Stmt* thenBranch = statement();
    Stmt::Stmt* elseBranch = nullptr;

    if (match(Lexing::ELSE)) elseBranch = statement();

    return new Stmt::If(condition, thenBranch, elseBranch);
}

Stmt::Stmt* Parser::whileStatement() {
    consume(Lexing::LEFT_PAREN, " after \"while\"");
    Expr::Expr* condition = expression();
    consume(Lexing::RIGHT_PAREN, " after while condition");

    Stmt::Stmt* body = statement();

    return new Stmt::While(condition, body);
}

Stmt::Stmt* Parser::forStatement() {
    // TODO
    return nullptr;
}

Stmt::Stmt* Parser::blockStatement() {
    std::vector<Stmt::Stmt*> contents;
    while (!match(Lexing::RIGHT_BRACE)) {
        if (check(Lexing::END_OF_FILE)) {
            error("Unterminated block");
            break;
        }
        Stmt::Stmt* stmt = declaration();
        contents.push_back(stmt);
    }

    return new Stmt::Block(contents);
}