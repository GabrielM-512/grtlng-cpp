#include "parsing.h"

#include <iostream>
#include <utility>

#include "expressions.h"
#include "../compiler.h"

using namespace Parsing;

Parser::Parser(std::vector<Lexing::Token>& tokens, Error::ErrorHandler& handler) : tokens(tokens),
    errorHandler(handler) {
    currentToken = 0;
    hadError = false;
    hadFatalError = false;

    current = tokens.at(0);
    previous = (Lexing::Token) {.type = Lexing::ERROR, .line = 1, .position = 0, .data = {nullptr}};

    advance();

    Expressions::registerExpressionParselets(*this);
}

std::vector<Stmt::Stmt*> Parser::parse() {

    std::vector<Stmt::Stmt*> tree;
    while (!isAtEnd()) {
        try {
            Stmt::Stmt* stmt;

            if (matchTypeIdent()) stmt = localDeclarationStatement();
            else stmt = statement();

            tree.push_back(stmt);
        } catch (Compiler::CompileError& e) {
            if (e.makeError) {
                errorAt(e.token, e.message, "", false);
            }
        }
    }

    errorHandler.printErrors();

    return tree;
}

/*
    PPPP      A     RRRR     SSSS   EEEEE   L       EEEEE   TTTTT    SSSS
    P   P    A A    R   R   S       E       L       E         T     S
    PPPP     AAA    RRRR     SSS    EEEEE   L       EEEEE     T      SSS
    P       A   A   R  R        S   E       L       E         T         S
    P       A   A   R   R   SSSS    EEEEE   LLLLL   EEEEE     T     SSSS
*/

void Parser::registerPrefixParselet(PrefixParselet* parselet, Lexing::TokenType type) {
    prefixTable.insert({type, parselet});
}
void Parser::registerInfixParselet(InfixParselet* parselet, Lexing::TokenType type) {
    infixTable.insert({type, parselet});
}

PrefixParselet* Parser::getPrefixParselet(Lexing::TokenType type) const {
    auto parselet = prefixTable.find(type);

    return parselet == prefixTable.end() ? nullptr : parselet->second;

}
InfixParselet* Parser::getInfixParselet(Lexing::TokenType type) const {
    auto parselet = infixTable.find(type);

    return parselet == infixTable.end() ? nullptr : parselet->second;
}

int Parser::getPrecedence(Lexing::TokenType type) const {
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

Lexing::Token Parser::advance() {
    previous = current;
    while (true) {

        current = tokens[currentToken++];
        if (current.type != Lexing::ERROR) break;

        errorAtCurrent(current.data.name);
    }

    return previous;
}

bool Parser::consume(Lexing::TokenType type, const std::string &message) {
    if (check(type)) {
        if (!isAtEnd()) advance();
        return true;
    }

    throw errorAtCurrent("Expected " + Lexing::Token::toString(type) + message + ", got " + current.toString() + " instead");
    return false;
}

bool Parser::consume(Lexing::TokenType type) {
    return consume(type, "");
}

bool Parser::match(Lexing::TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Lexing::TokenType type) const {
    return peek().type == type;
}

Lexing::Token Parser::peek() const {
    return current;
}

bool Parser::isAtEnd() const {
    return currentToken >= tokens.size();
}

bool Parser::checkTypeIdent() const {

    using namespace Lexing;

    switch (peek().type) {
        case I8:
        case I16:
        case I32:
        case I64:
        case U8:
        case U16:
        case U32:
        case U64:
        case F32:
        case F64:
        case VOID:
            return true;

        default:
            return false;
    }
}
bool Parser::matchTypeIdent() {
    bool result = checkTypeIdent();
    if (result) advance();
    return result;
}


bool Parser::hadParseError() const {
    return hadError;
}

bool Parser::hadFatalParseError() const {
    return hadFatalError;
}

/*
    EEEEE   RRRR    RRRR     OOO    RRRR     SSSS
    E       R   R   R   R   O   O   R   R   S
    EEEEE   RRRR    RRRR    O   O   RRRR     SSS
    E       R  R    R  R    O   O   R  R        S
    EEEEE   R   R   R   R    OOO    R   R   SSSS
*/

Compiler::CompileError Parser::errorAt(Lexing::Token token, std::string message, std::string hint, bool fatal) {
    hadError = true;
    if (fatal) hadFatalError = true;
    errorHandler.compileError(std::move(message), std::move(hint), token);

    return Compiler::CompileError(message, token, false);
}

Compiler::CompileError Parser::fatalErrorAtCurrent(std::string message) {
    return errorAt(current, std::move(message), "", true);
}
Compiler::CompileError Parser::fatalError(std::string message) {
    return errorAt(previous, std::move(message), "", true);
}
Compiler::CompileError Parser::errorAtCurrent(std::string message) {
    return errorAt(current, std::move(message), "", false);
}
Compiler::CompileError Parser::error(std::string message) {
    return errorAt(previous, std::move(message), "", false);
}


Compiler::CompileError Parser::fatalErrorAtCurrent(std::string message, std::string hint) {
    return errorAt(current, std::move(message), std::move(hint), true);
}
Compiler::CompileError Parser::fatalError(std::string message, std::string hint) {
    return errorAt(previous, std::move(message), std::move(hint), true);
}
Compiler::CompileError Parser::errorAtCurrent(std::string message, std::string hint) {
    return errorAt(current, std::move(message), std::move(hint), false);
}
Compiler::CompileError Parser::error(std::string message, std::string hint) {
    return errorAt(previous, std::move(message), std::move(hint), false);
}