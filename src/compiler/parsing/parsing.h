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
        static constexpr int CONDITIONAL = 3;
        static constexpr int LOGICAL_OR = 4;
        static constexpr int LOGICAL_AND = 5;
        static constexpr int EQUALITY = 6;
        static constexpr int COMPARISON = 7;
        static constexpr int SUM = 8;
        static constexpr int PRODUCT = 9;
        static constexpr int UNARY = 10;
        static constexpr int CALL = 11;
    };

    class Parser {
        std::vector<Lexing::Token>& tokens;
        Lexing::Token current, previous;
        u32 currentToken;
        u16 loopCount;
        u16 switchCount;

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

        Stmt::VariableDeclaration *variableDeclaration(Lexing::Token dataType, const Lexing::Token &name);

        Stmt::Stmt* functionDeclaration(Lexing::Token dataType, const Lexing::Token& name);
        std::vector<Stmt::VariableDeclaration*> parseParameters();

        Stmt::Stmt* declaration();
        Stmt::Stmt* globalDeclaration();

        Stmt::Stmt* statement();

        Stmt::Print *printStatement();
        Stmt::Expression *expressionStatement();
        Stmt::VariableDeclaration *localDeclarationStatement();
        Stmt::If *ifStatement();
        Stmt::While *whileStatement();
        Stmt::Stmt* forStatement();
        Stmt::Block *blockStatement();

        Stmt::Return *returnStatement();

        Stmt::Break *breakStatement();
        Stmt::Continue *continueStatement();

        Stmt::Switch *switchStatement();
        Stmt::Case switchCase(std::optional<Expr::Expr *>, Lexing::Token exprToken);
        void synchronise(bool isGlobal);

        [[nodiscard]] bool hasLoop() const;
        [[nodiscard]] bool hasSwitch() const;

    public:
        bool consume(Lexing::TokenType type, const std::string &message);
        bool consume(Lexing::TokenType type);
        bool match(Lexing::TokenType type);
        [[nodiscard]] bool check(Lexing::TokenType type) const;

        static Lexing::Token fakeToken();

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

        Compiler::CompileError errorAt(Lexing::Token token, const std::string& message, std::string hint, bool fatal);

        Compiler::CompileError fatalErrorAtCurrent(const std::string& message);
        Compiler::CompileError fatalError(const std::string& message);
        Compiler::CompileError errorAtCurrent(const std::string& message);
        Compiler::CompileError error(const std::string& message);

        Compiler::CompileError fatalErrorAtCurrent(const std::string& message, std::string hint);
        Compiler::CompileError fatalError(const std::string& message, std::string hint);
        Compiler::CompileError errorAtCurrent(const std::string& message, std::string hint);
        Compiler::CompileError error(const std::string& message, std::string hint);
    };

}
