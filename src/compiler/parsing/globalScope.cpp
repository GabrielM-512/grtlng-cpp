#include "parsing.h"
#include "../../AST/stmt.h"

using namespace Parsing;
// leaves us sitting on the (hopefully existent) '{'
std::vector<Stmt::VariableDeclaration*> Parser::parseParameters() {
    std::vector<Stmt::VariableDeclaration*> params;

    if (match(Lexing::RIGHT_PAREN)) {
        return params;
    }

    try {
        do {
            if (!matchTypeIdent()) {
                throw errorAtCurrent("Expected parameter type, got " + peek().toString() + " instead");
            }

            Lexing::TokenType dataType = previous.type;

            Lexing::Token name = current;

            name.data.name = "";

            if (match(Lexing::IDENTIFIER)) {
                name = previous;
            }

            params.push_back(new Stmt::VariableDeclaration(dataType, name, nullptr));

        } while (match(Lexing::COMMA));
    } catch (Error::CompileError&) {
        while (!check(Lexing::RIGHT_PAREN) && !check(Lexing::END_OF_FILE)) advance(); // skip remaining faulty parameters
    }

    consume(Lexing::RIGHT_PAREN, " after function parameters");

    return params;
}

Stmt::Stmt* Parser::functionDeclaration(Lexing::TokenType dataType, const Lexing::Token& name) {
    std::vector<Stmt::VariableDeclaration*> parameters = parseParameters();

    if (!match(Lexing::LEFT_BRACE)) {
        throw errorAtCurrent("Functions must have a block as their body");
    }

    Stmt::Block* body = blockStatement();

    return new Stmt::Function(dataType, name, parameters, body);

    return nullptr;
}

Stmt::Stmt* Parser::globalDeclaration() {
    if (!matchTypeIdent()) {
        throw errorAtCurrent("Expected Function or Variable declaration");
    }

    Lexing::TokenType dataType = previous.type;

    consume(Lexing::IDENTIFIER, " after declaration datatype");

    const Lexing::Token name = previous;

    if (match(Lexing::LEFT_PAREN)) return functionDeclaration(dataType, name);
    return variableDeclaration(dataType, name);
}
