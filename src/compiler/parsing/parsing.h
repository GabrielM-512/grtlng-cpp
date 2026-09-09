#pragma once
#include <map>

#include "../lexing.h"
#include "../../error.h"
#include "../../AST/expr.h"
#include "../../AST/stmt.h"

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
        bool hadFatalError;

        std::map<Lexing::Tokens::TokenType, PrefixParselet*> prefixTable;
        std::map<Lexing::Tokens::TokenType, InfixParselet*> infixTable;

        Error::ErrorHandler& errorHandler;


        [[nodiscard]] bool isAtEnd() const;
        Lexing::Tokens::Token advance();
        [[nodiscard]] Lexing::Tokens::Token peek() const;

        [[nodiscard]] bool checkTypeIdent() const;
        bool matchTypeIdent();

        [[nodiscard]] PrefixParselet* getPrefixParselet(Lexing::Tokens::TokenType type) const;
        [[nodiscard]] InfixParselet* getInfixParselet(Lexing::Tokens::TokenType type) const;

        [[nodiscard]] int getPrecedence() const;
        [[nodiscard]] int getPrecedence(Lexing::Tokens::TokenType type) const;

        Stmt::Stmt* statement();
        Stmt::Stmt* printStatement();
        Stmt::Stmt* expressionStatement();
        Stmt::Stmt* localDeclarationStatement();
        
        void errorAt(Lexing::Tokens::Token token, std::string message, std::string hint, bool fatal);

    public:
        bool consume(Lexing::Tokens::TokenType type, const std::string &message);
        bool consume(Lexing::Tokens::TokenType type);
        bool match(Lexing::Tokens::TokenType type);

        explicit Parser(std::vector<Lexing::Tokens::Token>& tokens, Error::ErrorHandler& handler);

        std::vector<Stmt::Stmt *> parse();
        [[nodiscard]] bool hadParseError() const;
        [[nodiscard]] bool hadFatalParseError() const;

        void registerPrefixParselet(PrefixParselet* parselet, Lexing::Tokens::TokenType type);
        void registerInfixParselet(InfixParselet* parselet, Lexing::Tokens::TokenType type);

        Expr::Expr* parseExpression(int precedence);
        Expr::Expr* parseExprPrec();
        Expr::Expr* parseExprPrecRight();
        Expr::Expr* expression();

        void fatalErrorAtCurrent(std::string message);
        void fatalError(std::string message);
        void errorAtCurrent(std::string message);
        void error(std::string message);

        void fatalErrorAtCurrent(std::string message, std::string hint);
        void fatalError(std::string message, std::string hint);
        void errorAtCurrent(std::string message, std::string hint);
        void error(std::string message, std::string hint);
    };

}
