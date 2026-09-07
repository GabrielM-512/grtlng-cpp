#include "expressions.h"

/*
    PPPP    RRRR    EEEEE   FFFFF    III    X   X
    P   P   R   R   E       F        III     X X
    PPPP    RRRR    EEEEE   FFFFF    III      X
    P       R  R    E       F        III     X X
    P       R   R   EEEEE   F        III    X   X
*/

class IdentifierParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser&, Lexing::Tokens::Token token) override {
        return new Expr::Identifier(token.data.name);
    }
};

class NumberParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser&, Lexing::Tokens::Token token) override {
        return new Expr::Number(token.data.number);
    }
};

class UnaryParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser& parser, Lexing::Tokens::Token token) override {
        Expr::Expr* operand = parser.parseExprPrecRight();
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

class BinaryParselet : public Parsing::InfixParselet {
    int precedence;
public:
    BinaryParselet(int precedence): precedence(precedence) {}

    Expr::Expr* parse(Parsing::Parser &parser, Expr::Expr *left, Lexing::Tokens::Token token) override {
        Expr::Expr* right = parser.parseExprPrec();
        return new Expr::Binary(left, token.type, right);
    }
    int getPrecedence() override {return precedence;}
};

void registerUnaryParselet(Parsing::Parser& parser, Lexing::Tokens::TokenType type) {
    parser.registerPrefixParselet(new UnaryParselet(), type);
}

void registerBinaryParselet(Parsing::Parser& parser, Lexing::Tokens::TokenType type, int precedence) {
    parser.registerInfixParselet(new BinaryParselet(precedence), type);
}

void Expressions::registerExpressionParselets(Parsing::Parser &parser) {
    parser.registerPrefixParselet(new IdentifierParselet(), Lexing::Tokens::IDENTIFIER);
    parser.registerPrefixParselet(new NumberParselet(), Lexing::Tokens::NUMBER);

    registerUnaryParselet(parser, Lexing::Tokens::PLUS);
    registerUnaryParselet(parser, Lexing::Tokens::MINUS);

    registerBinaryParselet(parser, Lexing::Tokens::PLUS, Parsing::Precedence::SUM);
    registerBinaryParselet(parser, Lexing::Tokens::MINUS, Parsing::Precedence::SUM);
    registerBinaryParselet(parser, Lexing::Tokens::SLASH, Parsing::Precedence::PRODUCT);
    registerBinaryParselet(parser, Lexing::Tokens::STAR, Parsing::Precedence::PRODUCT);
}