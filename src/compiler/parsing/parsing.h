#pragma once
#include <map>

#include "../compiler.h"
#include "../lexing.h"
#include "../../error.h"
#include "../../AST/expr.h"
#include "../../AST/stmt.h"

namespace Parsing {

    class PrefixParselet;
    class InfixParselet;

    class Precedence {
    public:
        static constexpr int LIMIT = 1;
        static constexpr int ASSIGNMENT = 2;
        static constexpr int LOGICAL_OR = 3;
        static constexpr int LOGICAL_AND = 4;
        static constexpr int EQUALITY = 5;
        static constexpr int COMPARISON = 6;
        static constexpr int SUM = 7;
        static constexpr int PRODUCT = 8;
        static constexpr int UNARY = 9;
        static constexpr int CALL = 10;
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

        Compiler::CompileError errorAt(Lexing::Token token, std::string message, std::string hint, bool fatal);

        Compiler::CompileError fatalErrorAtCurrent(std::string message);
        Compiler::CompileError fatalError(std::string message);
        Compiler::CompileError errorAtCurrent(std::string message);
        Compiler::CompileError error(std::string message);

        Compiler::CompileError fatalErrorAtCurrent(std::string message, std::string hint);
        Compiler::CompileError fatalError(std::string message, std::string hint);
        Compiler::CompileError errorAtCurrent(std::string message, std::string hint);
        Compiler::CompileError error(std::string message, std::string hint);
    };

}
