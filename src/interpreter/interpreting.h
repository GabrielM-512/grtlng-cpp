#pragma once
#include "environment.h"
#include "../AST/expr.h"
#include "../AST/stmt.h"
#include "../error.h"
#include "../compiler/compiler.h"

namespace Interpreting {

    class Interpreter : public Expr::ExprVisitor, public Stmt::StmtVisitor {
        Environment global;
        Environment *current;

        void beginEnvironment();
        void endEnvironment();

        Value::Value evaluate(Expr::Expr* expr);
        void execute(Stmt::Stmt* stmt);

    public:
        Interpreter();
        ~Interpreter() override;

        void interpret(Compiler::CompileResult& program);

        ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override;
        ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override;
        ExprVisitResults visitNumberExpr(Expr::Number* expr) override;
        ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override;
        ExprVisitResults visitAssignExpr(Expr::Assign *expr) override;

        StmtVisitResults visitPrintStmt(Stmt::Print *stmt) override;
        StmtVisitResults visitExpressionStmt(Stmt::Expression *stmt) override;
        StmtVisitResults visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) override;
        StmtVisitResults visitIfStmt(Stmt::If *stmt) override;
        StmtVisitResults visitWhileStmt(Stmt::While *stmt) override;
    };

    Value::Value interpret(Compiler::CompileResult& program, Error::ErrorHandler& handler);
}
