#include "expressions.h"

/*
    PPPP    RRRR    EEEEE   FFFFF    III    X   X
    P   P   R   R   E       F        III     X X
    PPPP    RRRR    EEEEE   FFFFF    III      X
    P       R  R    E       F        III     X X
    P       R   R   EEEEE   F        III    X   X
*/

class identifierParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser&, Lexing::Tokens::Token token) override {
        return new Expr::Identifier(token.data.name);
    }
};

class numberParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser&, Lexing::Tokens::Token token) override {
        return new Expr::Number(token.data.number);
    }
};

class unaryParselet : public Parsing::PrefixParselet {
    Expr::Expr* parse(Parsing::Parser& parser, Lexing::Tokens::Token token) override {
        Expr::Expr* operand = parser.parseExpression();
        return new Expr::Unary(token.type, operand);
    }
};

/*
     III    N   N   FFFFF    III    X   X
     III    NN  N   F        III     X X
     III    N N N   FFFFF    III      X
     III    N  NN   F        III     X X
     III    N   N   F        III    X   X
*/

class binaryParselet : public Parsing::InfixParselet {
public:
    Expr::Expr* parse(Parsing::Parser &parser, Expr::Expr *left, Lexing::Tokens::Token token) override {
        Expr::Expr* right = parser.parseExpression();
        return new Expr::Binary(left, token.type, right);
    }
};

void registerUnaryParselet(Parsing::Parser& parser, Lexing::Tokens::TokenType type) {
    parser.registerPrefixParselet(new unaryParselet(), type);
}

void registerBinaryParselet(Parsing::Parser& parser, Lexing::Tokens::TokenType type) {
    parser.registerInfixParselet(new binaryParselet(), type);
}

void Expressions::registerExpressionParselets(Parsing::Parser &parser) {
    parser.registerPrefixParselet(new identifierParselet(), Lexing::Tokens::IDENTIFIER);
    parser.registerPrefixParselet(new numberParselet(), Lexing::Tokens::NUMBER);

    registerUnaryParselet(parser, Lexing::Tokens::PLUS);
    registerUnaryParselet(parser, Lexing::Tokens::MINUS);

    registerBinaryParselet(parser, Lexing::Tokens::PLUS);
    registerBinaryParselet(parser, Lexing::Tokens::MINUS);
    registerBinaryParselet(parser, Lexing::Tokens::SLASH);
    registerBinaryParselet(parser, Lexing::Tokens::STAR);
}