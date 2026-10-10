#pragma once
#include "compileStructs.h"
#include "../AST/expr.h"



namespace Classifying {
    // used for checking whether an expression is constexpr qualified, a variable is static, etc
    class Classifier : Expr::ExprVisitor {
        [[nodiscard]] structs::test  checkExpr(Expr::Expr* expr);
    public:
        bool isConstexprQualified(Expr::Expr* expr);
        bool isStaticInitialised(const std::string&);
        bool isConstVar(const std::string& name);

        void declareVar(Stmt::VariableDeclaration* var);

        ExprVisitResults visitAssignExpr(Expr::Assign *expr) override;
        ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override;
        ExprVisitResults visitCallExpr(Expr::Call *expr) override;
        ExprVisitResults visitConditionalExpr(Expr::Conditional *expr) override;
        ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override;
        ExprVisitResults visitLogicalExpr(Expr::Logical *expr) override;
        ExprVisitResults visitNumberExpr(Expr::Number *expr) override;
        ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override;
    };
}