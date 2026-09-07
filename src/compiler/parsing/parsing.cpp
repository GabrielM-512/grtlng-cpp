#include "parsing.h"

#include <iostream>

#include "expressions.h"

using namespace Parsing;

Parser::Parser(std::vector<Lexing::Tokens::Token>& tokens): tokens(tokens) {
    currentToken = 0;
    hadError = false;

    current = tokens.at(0);
    previous = (Lexing::Tokens::Token) {.type = Lexing::Tokens::ERROR, .line = 1, .position = 0, .data = {nullptr}};

    advance();

    Expressions::registerExpressionParselets(*this);
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
    U   U   TTTTT    III    L        SSSS
    U   U     T      III    L       S
    U   U     T      III    L        SSS
    U   U     T      III    L           S
     UUU      T      III    LLLLL   SSSS
*/

Lexing::Tokens::Token Parser::advance() {
    previous = current;
    while (true) {
        if (isAtEnd()) {
            current = tokens[tokens.size() - 1];
        } else {
            current = tokens[currentToken++];
        }

        if (current.type != Lexing::Tokens::ERROR) break;

        std::cerr << "Error on line " << current.line << ": " << current.data.name << std::endl;
    }

    return previous;
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
        // TODO: Proper error handling
        std::cerr << "Error on line " << token.line << ": Expected expression at " << token.toString() << std::endl;
        hadError = true;
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

Expr::Expr *Parser::parse() {
    return expression();
}