#pragma once

#include <variant>

#include "../compiler/lexing.h" 
#include  "../value.h"

namespace Expr {
    struct Binary;
    struct Unary;
    struct Number;
    struct Identifier;

    #define ExprVisitResults std::variant<std::string, Value::Value>

    class ExprVisitor {
    public:
        virtual ~ExprVisitor() = default;

        virtual ExprVisitResults visitBinaryExpr(Binary* expr) = 0;
        virtual ExprVisitResults visitUnaryExpr(Unary* expr) = 0;
        virtual ExprVisitResults visitNumberExpr(Number* expr) = 0;
        virtual ExprVisitResults visitIdentifierExpr(Identifier* expr) = 0;
    };

    struct Expr {
        virtual ~Expr() = default;
        virtual ExprVisitResults accept(ExprVisitor *visitor) = 0;
    };

    struct Binary: Expr {
        Expr* left;
        Lexing::Tokens::TokenType operatorType;
        Expr* right;

        explicit Binary(
            Expr* left = nullptr,
            Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR,
            Expr* right = nullptr
        ): left(left), operatorType(operatorType), right(right) {}

        ExprVisitResults accept(ExprVisitor* visitor) override {
            return visitor->visitBinaryExpr(this);
        }
    };

    struct Unary: Expr {
        Lexing::Tokens::TokenType operatorType;
        Expr* right;

        explicit Unary(
            Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR,
            Expr* right = nullptr
        ): operatorType(operatorType), right(right) {}

        ExprVisitResults accept(ExprVisitor* visitor) override {
            return visitor->visitUnaryExpr(this);
        }
    };

    struct Number: Expr {
        double value;

        explicit Number(
            double value = 0
        ): value(value) {}

        ExprVisitResults accept(ExprVisitor* visitor) override {
            return visitor->visitNumberExpr(this);
        }
    };

    struct Identifier: Expr {
        const char* target;

        explicit Identifier(
            const char* target
        ): target(target) {}

        ExprVisitResults accept(ExprVisitor* visitor) override {
            return visitor->visitIdentifierExpr(this);
        }
    };
}