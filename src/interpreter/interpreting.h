#pragma once
#include "environment.h"
#include "../AST/expr.h"
#include "../AST/stmt.h"
#include "../error.h"

namespace Interpreting {

    class Interpreter : public Expr::ExprVisitor, public Stmt::StmtVisitor {
        Environment global;
        Environment *current;

        void beginEnvironment();
        void endEnvironment();

        Value::Value evaluate(Expr::Expr* expr);

    public:
        Interpreter();
        ~Interpreter() override;

        Value::Value interpret(Expr::Expr* program);

        ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override;
        ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override;
        ExprVisitResults visitNumberExpr(Expr::Number* expr) override;
        ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override;
    };

    Value::Value interpret(Expr::Expr *program, Error::ErrorHandler& handler);
}
