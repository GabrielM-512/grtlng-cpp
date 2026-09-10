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
    Expr::Expr* parse(Parsing::Parser&, Lexing::Token& token) override {
        return new Expr::Identifier(token);
    }
};

class NumberParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser&, Lexing::Token& token) override {
        return new Expr::Number(token.data.number);
    }
};

class UnaryParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse(Parsing::Parser& parser, Lexing::Token& token) override {
        Expr::Expr* operand = parser.parseExprPrecRight();
        return new Expr::Unary(token.type, operand);
    }
};

class GroupingParselet : public Parsing::PrefixParselet {
public:
    Expr::Expr* parse (Parsing::Parser& parser, Lexing::Token&) override {
        Expr::Expr* node = parser.expression();
        parser.consume(Lexing::RIGHT_PAREN, "Expected ')'");

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

    Expr::Expr* parse(Parsing::Parser &parser, Expr::Expr *left, Lexing::Token& token) override {
        Expr::Expr* right = parser.parseExprPrec();
        return new Expr::Binary(left, token.type, right);
    }
    int getPrecedence() override {return precedence;}
};


/*
    PPPP      A     RRRR     SSSS   EEEEE   L       EEEEE   TTTTT           U   U   TTTTT   IIIII   L        SSSS
    P   P    A A    R   R   S       E       L       E         T             U   U     T       I     L       S
    PPPP     AAA    RRRR     SSS    EEEEE   L       EEEEE     T             U   U     T       I     L        SSS
    P       A   A   R  R        S   E       L       E         T             U   U     T       I     L           S
    P       A   A   R   R   SSSS    EEEEE   LLLLL   EEEEE     T              UUU      T     IIIII   LLLLL   SSSS
*/

void registerUnaryParselet(Parsing::Parser& parser, Lexing::TokenType type) {
    parser.registerPrefixParselet(new UnaryParselet(), type);
}

void registerBinaryParselet(Parsing::Parser& parser, Lexing::TokenType type, int precedence) {
    parser.registerInfixParselet(new BinaryParselet(precedence), type);
}

void Expressions::registerExpressionParselets(Parsing::Parser &parser) {
    parser.registerPrefixParselet(new IdentifierParselet(), Lexing::IDENTIFIER);
    parser.registerPrefixParselet(new NumberParselet(), Lexing::NUMBER);
    parser.registerPrefixParselet(new GroupingParselet(), Lexing::LEFT_PAREN);

    registerUnaryParselet(parser, Lexing::PLUS);
    registerUnaryParselet(parser, Lexing::MINUS);

    registerBinaryParselet(parser, Lexing::PLUS, Parsing::Precedence::SUM);
    registerBinaryParselet(parser, Lexing::MINUS, Parsing::Precedence::SUM);
    registerBinaryParselet(parser, Lexing::SLASH, Parsing::Precedence::PRODUCT);
    registerBinaryParselet(parser, Lexing::STAR, Parsing::Precedence::PRODUCT);
}


/*
    PPPP      A     RRRR     SSSS   IIIII   N   N    GGG            U   U   TTTTT   IIIII   L       IIIII   TTTTT   IIIII   EEEEE    SSSS
    P   P    A A    R   R   S         I     NN  N   G               U   U     T       I     L         I       T       I     E       S
    PPPP     AAA    RRRR     SSS      I     N N N   G  GG           U   U     T       I     L         I       T       I     EEEEE    SSS
    P       A   A   R  R        S     I     N  NN   G   G           U   U     T       I     L         I       T       I     E           S
    P       A   A   R   R   SSSS    IIIII   N   N    GGG             UUU      T     IIIII   LLLLL   IIIII     T     IIIII   EEEEE   SSSS
*/

using namespace Parsing;

Expr::Expr* Parser::parseExpression(int precedence) {
    Lexing::Token token = advance();

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