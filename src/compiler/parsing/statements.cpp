#include "parsing.h"
#include "../compiler.h"

using namespace Parsing;

Stmt::Stmt* Parser::statement() {
    if (match(Lexing::PRINT)) return printStatement();
    if (match(Lexing::IF)) return ifStatement();
    if (match(Lexing::WHILE)) return whileStatement();
    if (match(Lexing::FOR)) return forStatement();
    if (match(Lexing::LEFT_BRACE)) return blockStatement();
    if (match(Lexing::RETURN)) return returnStatement();

    return expressionStatement();
}

Stmt::Stmt* Parser::declaration() {
    if (matchTypeIdent()) return localDeclarationStatement();
    return statement();
}

Stmt::Print *Parser::printStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::SEMICOLON);
    return new Stmt::Print(expr);
}

Stmt::Expression *Parser::expressionStatement() {
    Expr::Expr* expr = expression();
    consume(Lexing::SEMICOLON);
    return new Stmt::Expression(expr);
}

Stmt::VariableDeclaration *Parser::variableDeclaration(Lexing::TokenType dataType, const Lexing::Token &name) {
    Expr::Expr* value = nullptr;

    if (match(Lexing::EQUALS)) {
        value = expression();
    }

    consume(Lexing::SEMICOLON, " after variable declaration");

    return new Stmt::VariableDeclaration(dataType, name, value);
}

Stmt::VariableDeclaration *Parser::localDeclarationStatement() {
    Lexing::TokenType dataType = previous.type;
    consume(Lexing::IDENTIFIER, " after data type");

    if (match(Lexing::LEFT_PAREN)) {
        throw error("Expected \"=\" or \";\", got \"(\" instead)", "Function declarations are only allowed in the global scope");
    }

    const Lexing::Token name = previous;

    return variableDeclaration(dataType, name);
}

Stmt::If *Parser::ifStatement() {
    consume(Lexing::LEFT_PAREN, " after \"if\"");
    Expr::Expr* condition = expression();
    consume(Lexing::RIGHT_PAREN, " after if condition");

    Stmt::Stmt* thenBranch = statement();
    Stmt::Stmt* elseBranch = nullptr;

    if (match(Lexing::ELSE)) elseBranch = statement();

    return new Stmt::If(condition, thenBranch, elseBranch);
}

Stmt::While *Parser::whileStatement() {
    consume(Lexing::LEFT_PAREN, " after \"while\"");
    Expr::Expr* condition = expression();
    consume(Lexing::RIGHT_PAREN, " after while condition");

    Stmt::Stmt* body = statement();

    return new Stmt::While(condition, body);
}

Stmt::Stmt* Parser::forStatement() {
    // desugar for loop to while loop
    consume(Lexing::LEFT_PAREN, " after \"while\"");
    Stmt::Stmt* initialiser = nullptr;

    if (!check(Lexing::SEMICOLON)) {
        // initialiser
        if (matchTypeIdent()) initialiser = localDeclarationStatement();
        else initialiser = expressionStatement();
    }

    Expr::Expr* condition;

    if(!check(Lexing::SEMICOLON)) {
        // condition
        condition = expression();
    } else {
        // always truthy
        condition = new Expr::Number(1);
    }

    consume(Lexing::SEMICOLON, " after condition");


    Expr::Expr* incrementer = nullptr;

    if (!check(Lexing::RIGHT_PAREN)) {
        incrementer = expression();
    }

    consume(Lexing::RIGHT_PAREN, " after incrementer clause");

    Stmt::Stmt* body = statement();


    if (incrementer != nullptr) {
        body = new Stmt::Block({body, new Stmt::Expression(incrementer)});
    }

    body = new Stmt::While(condition, body);

    if (initialiser != nullptr) {
        body = new Stmt::Block({initialiser, body});
    }

    return body;
}

Stmt::Block *Parser::blockStatement() {
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

Stmt::Return *Parser::returnStatement() {
    Expr::Expr* value = nullptr;

    if (!check(Lexing::SEMICOLON))
        value = expression();

    consume(Lexing::SEMICOLON, " after return value");
    return new Stmt::Return(value);
}