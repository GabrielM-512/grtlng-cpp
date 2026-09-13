#pragma once
#include "parsing.h"

class Parsing::PrefixParselet {
public:
    virtual ~PrefixParselet() = default;

    virtual Expr::Expr* parse(Parser& parser, Lexing::Token& token) = 0;
};

class Parsing::InfixParselet {
    int precedence;
public:
    virtual ~InfixParselet() = default;

    InfixParselet(int precedence) : precedence(precedence) {}

    virtual Expr::Expr* parse(Parser& parser, Expr::Expr* left, Lexing::Token& token) = 0;
    [[nodiscard]] int getPrecedence() const {return precedence;}
};

namespace Expressions {
    void registerExpressionParselets(Parsing::Parser& parser);
}