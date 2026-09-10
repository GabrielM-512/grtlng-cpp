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
        std::vector<Lexing::Token>& tokens;
        Lexing::Token current, previous;
        u32 currentToken;

        bool hadError;
        bool hadFatalError;

        std::map<Lexing::TokenType, PrefixParselet*> prefixTable;
        std::map<Lexing::TokenType, InfixParselet*> infixTable;

        Error::ErrorHandler& errorHandler;


        [[nodiscard]] bool isAtEnd() const;
        Lexing::Token advance();
        [[nodiscard]] Lexing::Token peek() const;

        [[nodiscard]] bool checkTypeIdent() const;
        bool matchTypeIdent();

        [[nodiscard]] PrefixParselet* getPrefixParselet(Lexing::TokenType type) const;
        [[nodiscard]] InfixParselet* getInfixParselet(Lexing::TokenType type) const;

        [[nodiscard]] int getPrecedence() const;
        [[nodiscard]] int getPrecedence(Lexing::TokenType type) const;

        Stmt::Stmt* statement();
        Stmt::Stmt* printStatement();
        Stmt::Stmt* expressionStatement();
        Stmt::Stmt* localDeclarationStatement();
        
        void errorAt(Lexing::Token token, std::string message, std::string hint, bool fatal);

    public:
        bool consume(Lexing::TokenType type, const std::string &message);
        bool consume(Lexing::TokenType type);
        bool match(Lexing::TokenType type);

        explicit Parser(std::vector<Lexing::Token>& tokens, Error::ErrorHandler& handler);

        std::vector<Stmt::Stmt *> parse();
        [[nodiscard]] bool hadParseError() const;
        [[nodiscard]] bool hadFatalParseError() const;

        void registerPrefixParselet(PrefixParselet* parselet, Lexing::TokenType type);
        void registerInfixParselet(InfixParselet* parselet, Lexing::TokenType type);

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
