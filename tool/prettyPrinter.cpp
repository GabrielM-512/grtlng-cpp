#include "prettyPrinter.h"
#include "../src/AST/expr.h"

class PrettyPrinter : public Expr::ExprVisitor {
public:

    static std::string operatorString(Lexing::TokenType type) {
        return Lexing::Token::toString(type).substr(1, 1);
    }

    static std::string operatorString(Lexing::Token token) {
        return operatorString(token.type);
    }

    ExprVisitResults visitBinaryExpr(Expr::Binary* expr) override {
        return "( " + std::get<std::string> (expr->left->accept(this)) + " " +
                    operatorString(expr->operatorType) + " " +
                    std::get<std::string> (expr->right->accept(this)) + " )";
    }

    ExprVisitResults visitUnaryExpr(Expr::Unary* expr) override {
        return "( " + operatorString(expr->operatorType) +
            std::get<std::string> (expr->right->accept(this)) + " )";
    }

    ExprVisitResults visitNumberExpr(Expr::Number* expr) override {
        return std::to_string(expr->value);
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier* expr) override {
        return expr->target.data.name;
    }

    ExprVisitResults visitAssignExpr(Expr::Assign *expr) override {
        return "( " + std::string(expr->name.data.name) + " = "
                    + std::get<std::string> (expr->value->accept(this)) + " )";
    }

    std::string print(Expr::Expr* node) {
        return std::get<std::string>(node->accept(this));
    }
};

std::string Printer::print(Expr::Expr *tree) {
    PrettyPrinter printer;
    return printer.print(tree);
}
