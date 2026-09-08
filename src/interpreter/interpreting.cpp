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

    Value::Value evaluate(Expr::Expr* expr) {
        return get<Value::Value>(expr->accept(this));
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

    ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override {
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

    ExprVisitResults visitNumberExpr(Expr::Number* expr) override {
        return expr->value;
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override {
        return current->getVar(expr->target);
    }

    Value::Value interpret(Expr::Expr* program) {
        return evaluate(program);
    }
};

Value::Value Interpreting::interpret(Expr::Expr *program) {
    Interpreter interpreter;


    double returnCode = interpreter.interpret(program);

    return returnCode;

}
