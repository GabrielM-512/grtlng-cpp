#include "expressions.h"

/*
    PPPP    RRRR    EEEEE   FFFFF   IIIII   X   X
    P   P   R   R   E       F         I      X X
    PPPP    RRRR    EEEEE   FFFFF     I       X
    P       R  R    E       F         I      X X
    P       R   R   EEEEE   F       IIIII   X   X
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

class GroupingParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse (Parsing::Parser& parser, Lexing::Tokens::Token) override {
        Expr::Expr* node = parser.expression();
        parser.consume(Lexing::Tokens::RIGHT_PAREN, "Expected ')'");

        return node;
    }
};

/*
    IIIII   N   N   FFFFF   IIIII   X   X
      I     NN  N   F         I      X X
      I     N N N   FFFFF     I       X
      I     N  NN   F         I      X X
    IIIII   N   N   F       IIIII   X   X
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
    parser.registerPrefixParselet(new GroupingParselet(), Lexing::Tokens::LEFT_PAREN);

    registerUnaryParselet(parser, Lexing::Tokens::PLUS);
    registerUnaryParselet(parser, Lexing::Tokens::MINUS);

    registerBinaryParselet(parser, Lexing::Tokens::PLUS, Parsing::Precedence::SUM);
    registerBinaryParselet(parser, Lexing::Tokens::MINUS, Parsing::Precedence::SUM);
    registerBinaryParselet(parser, Lexing::Tokens::SLASH, Parsing::Precedence::PRODUCT);
    registerBinaryParselet(parser, Lexing::Tokens::STAR, Parsing::Precedence::PRODUCT);
}