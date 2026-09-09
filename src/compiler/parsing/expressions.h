#pragma once
#include "parsing.h"

class Parsing::PrefixParselet {
public:
    virtual ~PrefixParselet() = default;

    virtual Expr::Expr* parse(Parser& parser, Lexing::Tokens::Token& token) = 0;
};

class Parsing::InfixParselet {
public:
    virtual ~InfixParselet() = default;

    virtual Expr::Expr* parse(Parser& parser, Expr::Expr* left, Lexing::Tokens::Token& token) = 0;
    virtual int getPrecedence() = 0;
};

namespace Expressions {
    void registerExpressionParselets(Parsing::Parser& parser);
}