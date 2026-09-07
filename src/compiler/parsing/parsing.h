#pragma once
#include <map>

#include "../lexing.h"
#include "../../AST/expr.h"

namespace Parsing {

    class PrefixParselet;
    class InfixParselet;

    class Precedence {
    public:
        static constexpr int ASSIGNMENT = 1;
        static constexpr int LOGICAL_OR = 2;
        static constexpr int LOGICAL_AND = 3;
        static constexpr int EQUALITY = 4;
        static constexpr int COMPARISON = 5;
        static constexpr int SUM = 6;
        static constexpr int PRODUCT = 7;
        static constexpr int UNARY = 8;
        static constexpr int CALL = 9;
    };

    class Parser {
        std::vector<Lexing::Tokens::Token>& tokens;
        Lexing::Tokens::Token current, previous;
        u32 currentToken;

        bool hadError;

        std::map<Lexing::Tokens::TokenType, PrefixParselet*> prefixTable;
        std::map<Lexing::Tokens::TokenType, InfixParselet*> infixTable;


        [[nodiscard]] bool isAtEnd() const;
        Lexing::Tokens::Token advance();
        [[nodiscard]] Lexing::Tokens::Token peek() const;

        [[nodiscard]] PrefixParselet* getPrefixParselet(Lexing::Tokens::TokenType type) const;
        [[nodiscard]] InfixParselet* getInfixParselet(Lexing::Tokens::TokenType type) const;

        [[nodiscard]] int getPrecedence() const;
        [[nodiscard]] int getPrecedence(Lexing::Tokens::TokenType type) const;

    public:
        bool consume(Lexing::Tokens::TokenType type, const std::string &message);

        explicit Parser(std::vector<Lexing::Tokens::Token>& tokens);
        Expr::Expr* parse();
        [[nodiscard]] bool hadParseError() const;

        void registerPrefixParselet(PrefixParselet* parselet, Lexing::Tokens::TokenType type);
        void registerInfixParselet(InfixParselet* parselet, Lexing::Tokens::TokenType type);

        Expr::Expr* parseExpression(int precedence);
        Expr::Expr* parseExprPrec();
        Expr::Expr* parseExprPrecRight();
        Expr::Expr* expression();
    };

}
