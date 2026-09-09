#include "parsing.h"

#include <iostream>
#include <utility>

#include "expressions.h"

using namespace Parsing;

Parser::Parser(std::vector<Lexing::Tokens::Token>& tokens, Error::ErrorHandler& handler) : tokens(tokens),
    errorHandler(handler) {
    currentToken = 0;
    hadError = false;

    current = tokens.at(0);
    previous = (Lexing::Tokens::Token) {.type = Lexing::Tokens::ERROR, .line = 1, .position = 0, .data = {nullptr}};

    advance();

    Expressions::registerExpressionParselets(*this);
}

void Parser::errorAt(Lexing::Tokens::Token token, std::string message, std::string hint, bool fatal) {
    hadError = true;
    if (fatal) hadFatalError = true;
    errorHandler.compileError(std::move(message), std::move(hint), token);
}

/*
    PPPP      A     RRRR     SSSS   EEEEE   L       EEEEE   TTTTT    SSSS
    P   P    A A    R   R   S       E       L       E         T     S
    PPPP     AAA    RRRR     SSS    EEEEE   L       EEEEE     T      SSS
    P       A   A   R  R        S   E       L       E         T         S
    P       A   A   R   R   SSSS    EEEEE   LLLLL   EEEEE     T     SSSS
*/

void Parser::registerPrefixParselet(PrefixParselet* parselet, Lexing::Tokens::TokenType type) {
    prefixTable.insert({type, parselet});
}
void Parser::registerInfixParselet(InfixParselet* parselet, Lexing::Tokens::TokenType type) {
    infixTable.insert({type, parselet});
}

PrefixParselet* Parser::getPrefixParselet(Lexing::Tokens::TokenType type) const {
    auto parselet = prefixTable.find(type);

    return parselet == prefixTable.end() ? nullptr : parselet->second;

}
InfixParselet* Parser::getInfixParselet(Lexing::Tokens::TokenType type) const {
    auto parselet = infixTable.find(type);

    return parselet == infixTable.end() ? nullptr : parselet->second;
}

int Parser::getPrecedence(Lexing::Tokens::TokenType type) const {
    InfixParselet* parselet = getInfixParselet(type);
    if (parselet != nullptr) return parselet->getPrecedence();

    return 0;
}

int Parser::getPrecedence() const {
    return getPrecedence(peek().type);
}

/*
    U   U   TTTTT   IIIII   L        SSSS
    U   U     T       I     L       S
    U   U     T       I     L        SSS
    U   U     T       I     L           S
     UUU      T     IIIII   LLLLL   SSSS
*/

Lexing::Tokens::Token Parser::advance() {
    previous = current;
    while (true) {

        current = tokens[currentToken++];
        if (current.type != Lexing::Tokens::ERROR) break;

        errorAtCurrent(current.data.name);
    }

    return previous;
}

bool Parser::consume(Lexing::Tokens::TokenType type, const std::string &message) {
    if (peek().type == type) {
        if (!isAtEnd()) advance();
        return true;
    }

    errorAtCurrent("Expected " + Lexing::Tokens::Token::toString(type) + message + ", got " + current.toString() + " instead");
    return false;
}

bool Parser::consume(Lexing::Tokens::TokenType type) {
    return consume(type, "");
}

bool Parser::match(Lexing::Tokens::TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Lexing::Tokens::Token Parser::peek() const {
    return current;
}

bool Parser::isAtEnd() const {
    return currentToken >= tokens.size();
}

Expr::Expr* Parser::parseExpression(int precedence) {
    Lexing::Tokens::Token token = advance();

    PrefixParselet* prefix = getPrefixParselet(token.type);

    if (prefix == nullptr) {
        errorAt(token, "Expected expression", "", false);
        return nullptr;
    }

    Expr::Expr* left = prefix->parse(*this, token);

    while (precedence < getPrecedence()) {
        token = advance();

        InfixParselet* infix = getInfixParselet(token.type);
        left = infix->parse(*this, left, token);
    }

    return left;

}

Expr::Expr* Parser::parseExprPrec() {
    return parseExpression(getPrecedence(previous.type));
}

Expr::Expr* Parser::parseExprPrecRight() {
    return parseExpression(getPrecedence(previous.type) - 1);
}

Expr::Expr* Parser::expression() {
    return parseExpression(Precedence::ASSIGNMENT);
}


bool Parser::hadParseError() const {
    return hadError;
}

std::vector<Stmt::Stmt*> Parser::parse() {

    std::vector<Stmt::Stmt*> tree;
    while (!isAtEnd()) {
        Stmt::Stmt* stmt = statement();
        tree.push_back(stmt);
    }

    errorHandler.printErrors();

    return tree;
}

void Parser::fatalErrorAtCurrent(std::string message) {
    errorAt(current, std::move(message), "", true);
}
void Parser::fatalError(std::string message) {
    errorAt(previous, std::move(message), "", true);
}
void Parser::errorAtCurrent(std::string message) {
    errorAt(current, std::move(message), "", false);
}
void Parser::error(std::string message) {
    errorAt(previous, std::move(message), "", false);
}


void Parser::fatalErrorAtCurrent(std::string message, std::string hint) {
    errorAt(current, std::move(message), std::move(hint), true);
}
void Parser::fatalError(std::string message, std::string hint) {
    errorAt(previous, std::move(message), std::move(hint), true);
}
void Parser::errorAtCurrent(std::string message, std::string hint) {
    errorAt(current, std::move(message), std::move(hint), false);
}
void Parser::error(std::string message, std::string hint) {
    errorAt(previous, std::move(message), std::move(hint), false);
}