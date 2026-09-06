#pragma once

#include "../compiler/lexing.h"

namespace Expr {
    struct Binary;
    struct Unary;
    struct Number;
    struct Identifier;

    class ExprVisitor {
    public:
        virtual ~ExprVisitor() = default;

        virtual void visitBinaryExpr(const Binary* expr) const = 0;
        virtual void visitUnaryExpr(const Unary* expr) const = 0;
        virtual void visitNumberExpr(const Number* expr) const = 0;
        virtual void visitIdentifierExpr(const Identifier* expr) const = 0;
    };

    struct Expr {
        virtual ~Expr() = default;
        virtual void accept(ExprVisitor *visitor) = 0;
    };

    struct Binary: Expr {
        const Expr* left;
        Lexing::Tokens::TokenType operatorType;
        const Expr* right;

        explicit Binary(
            const Expr* left = nullptr,
            Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR,
            const Expr* right = nullptr
        ): left(left), operatorType(operatorType), right(right) {}

        void accept(ExprVisitor* visitor) override {
            visitor->visitBinaryExpr(this);
        }
    };

    struct Unary: Expr {
        Lexing::Tokens::TokenType operatorType;
        const Expr* right;

        explicit Unary(
            Lexing::Tokens::TokenType operatorType = Lexing::Tokens::ERROR,
            const Expr* right = nullptr
        ): operatorType(operatorType), right(right) {}

        void accept(ExprVisitor* visitor) override {
            visitor->visitUnaryExpr(this);
        }
    };

    struct Number: Expr {
        double value;

        explicit Number(
            double value = 0
        ): value(value) {}

        void accept(ExprVisitor* visitor) override {
            visitor->visitNumberExpr(this);
        }
    };

    struct Identifier: Expr {
        const char* target;

        explicit Identifier(
            const char* target
        ): target(target) {}

        void accept(ExprVisitor* visitor) override {
            visitor->visitIdentifierExpr(this);
        }
    };
}