#include "interpreting.h"

#include <cmath>

using namespace Interpreting;

class Interpreter : public Expr::ExprVisitor {
public:
    ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override {
        Value::Value left = get<Value::Value>(expr->left->accept(this));
        Value::Value right = get<Value::Value>(expr->right->accept(this));

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

    ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override {
        Value::Value operand = get<Value::Value>(expr->right->accept(this));
        switch (expr->operatorType) {
            case Lexing::Tokens::PLUS:
                return operand;
            case Lexing::Tokens::MINUS:
                return -operand;
            default:
                return 0.0;
        }
    }

    ExprVisitResults visitNumberExpr(Expr::Number* expr) override {
        return expr->value;
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier *) override {
        return INFINITY;
    }
};

Value::Value Interpreting::interpret(Expr::Expr *program) {
    Interpreter interpreter;


    double returnCode = get<Value::Value>(program->accept(&interpreter));

    return returnCode;

}
