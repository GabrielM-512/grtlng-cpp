#pragma once
#include <map>

#include "../lexing.h"
#include "../../AST/expr.h"

namespace Parsing {

    class PrefixParselet;
    class InfixParselet;

    class Parser {
        std::vector<Lexing::Tokens::Token>& tokens;
        Lexing::Tokens::Token current, previous;
        u32 currentToken;

        bool hadError;

        std::map<Lexing::Tokens::TokenType, PrefixParselet*> prefixTable;
        std::map<Lexing::Tokens::TokenType, InfixParselet*> infixTable;


        bool isAtEnd() const;
        void advance();

        PrefixParselet* getPrefixParselet(Lexing::Tokens::TokenType type);
        InfixParselet* getInfixParselet(Lexing::Tokens::TokenType type) const;

    public:
        explicit Parser(std::vector<Lexing::Tokens::Token>& tokens);
        Expr::Expr* parse();
        bool hadParseError() const;

        void registerPrefixParselet(PrefixParselet* parselet, Lexing::Tokens::TokenType type);
        void registerInfixParselet(InfixParselet* parselet, Lexing::Tokens::TokenType type);

        Expr::Expr* parseExpression();
    };

}
