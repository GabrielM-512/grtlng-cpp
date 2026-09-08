#include "interpreting.h"
#include "environment.h"

using namespace Interpreting;

class Interpreter : public Expr::ExprVisitor {
    Environment global;
    Environment *current;

    void beginEnvironment() {
        current = new Environment(current);
    }

    void endEnvironment() {
        Environment* restore = current->enclosing;
        delete current;
        current = restore;
    }

public:

    Interpreter() {
        global = Environment();
        current = &global;
    }

    ~Interpreter() override {
        while (current->enclosing != nullptr) {
            endEnvironment();
        }
    }

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

    ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override {
        return current->getVar(expr->target);
    }
};

Value::Value Interpreting::interpret(Expr::Expr *program) {
    Interpreter interpreter;


    double returnCode = get<Value::Value>(program->accept(&interpreter));

    return returnCode;

}
