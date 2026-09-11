#include "interpreting.h"

using namespace Interpreting;

Value::Value Interpreter::evaluate(Expr::Expr* expr) {
    return get<Value::Value>(expr->accept(this));
}

/*
    EEEEE   X   X   PPPP    RRRR    EEEEE    SSSS    SSSS   IIIII    OOO    N   N           V   V   IIIII    SSSS   IIIII   TTTTT    OOO    RRRR     SSSS
    E        X X    P   P   R   R   E       S       S         I     O   O   NN  N           V   V     I     S         I       T     O   O   R   R   S
    EEEEE     X     PPPP    RRRR    EEEEE    SSS     SSS      I     O   O   N N N            V V      I      SSS      I       T     O   O   RRRR     SSS
    E        X X    P       R  R    E           S       S     I     O   O   N  NN            V V      I         S     I       T     O   O   R  R        S
    EEEEE   X   X   P       R   R   EEEEE   SSSS    SSSS    IIIII    OOO    N   N             V     IIIII   SSSS    IIIII     T      OOO    R   R   SSSS
*/

ExprVisitResults Interpreter::visitBinaryExpr(Expr::Binary *expr) {
    Value::Value left = evaluate(expr->left);
    Value::Value right = evaluate(expr->right);

#define OPERATION(type, op) case type: return left op right

    switch (expr->operatorType) {
        OPERATION(Lexing::PLUS, +);
        OPERATION(Lexing::MINUS, -);
        OPERATION(Lexing::STAR, *);
        OPERATION(Lexing::SLASH, /);

        case Lexing::MORE: return left > right ? 1.0 : 0.0f;
        case Lexing::MORE_EQUALS: return left >= right ? 1.0 : 0.0f;
        case Lexing::LESS: return left < right ? 1.0 : 0.0f;
        case Lexing::LESS_EQUALS: return left <= right ? 1.0 : 0.0f;

        default:
            return 0.0f;
    }
}

ExprVisitResults Interpreter::visitUnaryExpr(Expr::Unary *expr) {
    Value::Value operand = evaluate(expr->right);
    switch (expr->operatorType) {
        case Lexing::PLUS:
            return operand;
        case Lexing::MINUS:
            return -operand;
        default:
            return 0.0;
    }
}

ExprVisitResults Interpreter::visitNumberExpr(Expr::Number* expr) {
    return expr->value;
}

ExprVisitResults Interpreter::visitIdentifierExpr(Expr::Identifier *expr) {
    return current->getVar(expr->target);
}

ExprVisitResults Interpreter::visitAssignExpr(Expr::Assign *expr) {
    Value::Value value = evaluate(expr->value);
    current->setVar(expr->name, value);
    return value;
}