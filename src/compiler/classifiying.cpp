#include "classifying.h"
#include "../AST/stmt.h"

using namespace Classifying;

static structs::test defaultStruct = {.isConstexpr = true};

structs::test combineValues(const structs::test& a, const structs::test& b) {
    return (structs::test) {.isConstexpr = a.isConstexpr && b.isConstexpr};
}

structs::test combineValues(const std::vector<structs::test*>& list) {
    structs::test returnValue = defaultStruct;

    for (structs::test *current : list) {
        returnValue = combineValues(returnValue, *current);
    }

    return returnValue;
}

structs::test Classifier::checkExpr(Expr::Expr *expr) {
    return std::get<structs::test> (expr->accept(this));
}

bool Classifier::isStaticInitialised(const std::string&) {
    return false; // TODO
}

bool Classifier::isConstexprQualified(Expr::Expr *expr) {
    structs::test values = checkExpr(expr);

    return values.isConstexpr;
}

bool Classifier::isConstVar(const std::string&) {
    return false;
}

ExprVisitResults Classifier::visitAssignExpr(Expr::Assign *) {
    return (structs::test) {.isConstexpr = false}; // TODO: check
}

ExprVisitResults Classifier::visitBinaryExpr(Expr::Binary *expr) {
    structs::test left = checkExpr(expr->left);
    structs::test right = checkExpr(expr->right);

    return combineValues(left, right);
}

ExprVisitResults Classifier::visitCallExpr(Expr::Call *expr) {

    return (structs::test) {.isConstexpr = false};

    // TODO: make calls actually constexpr-viable

    structs::test callee = checkExpr(expr->callee);

    for (Expr::Expr* current : expr->args) {
        callee = combineValues(callee, checkExpr(current));
    }

    return callee;
}

ExprVisitResults Classifier::visitConditionalExpr(Expr::Conditional *expr) {
    structs::test condition = checkExpr(expr->condition);
    structs::test left = checkExpr(expr->thenBranch);
    structs::test right = checkExpr(expr->elseBranch);

    return combineValues({&condition, &left, &right});
}

ExprVisitResults Classifier::visitIdentifierExpr(Expr::Identifier *expr) {
    bool isStatic = isStaticInitialised(expr->token.data.name);
    bool isConst = isConstVar(expr->token.data.name);

    return (structs::test) {.isConstexpr = isStatic && isConst};
}

ExprVisitResults Classifier::visitLogicalExpr(Expr::Logical *expr) {
    structs::test left = checkExpr(expr->left);
    structs::test right = checkExpr(expr->right);

    return combineValues(left, right);
}

ExprVisitResults Classifier::visitNumberExpr(Expr::Number *) {
    return (structs::test) {.isConstexpr = true};
}

ExprVisitResults Classifier::visitUnaryExpr(Expr::Unary *expr) {
    return checkExpr(expr->right);
}

void Classifier::declareVar(Stmt::VariableDeclaration*) {

}