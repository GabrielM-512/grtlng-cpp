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

    switch (expr->operatorType) {
        case Lexing::Tokens::PLUS:
            return left + right;
        case Lexing::Tokens::MINUS:
            return left - right;
        case Lexing::Tokens::STAR:
            return left * right;
        case Lexing::Tokens::SLASH:
            return left / right;
        default:
            return 0.0f;
    }
}

ExprVisitResults Interpreter::visitUnaryExpr(Expr::Unary *expr) {
    Value::Value operand = evaluate(expr->right);
    switch (expr->operatorType) {
        case Lexing::Tokens::PLUS:
            return operand;
        case Lexing::Tokens::MINUS:
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