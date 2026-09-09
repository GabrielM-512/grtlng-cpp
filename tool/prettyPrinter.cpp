#include "prettyPrinter.h"
#include "../src/AST/expr.h"

class prettyPrinter : public Expr::ExprVisitor {
public:

    static std::string operatorString(Lexing::Tokens::TokenType type) {
        return Lexing::Tokens::Token::toString(type).substr(1, 1);
    }

    static std::string operatorString(Lexing::Tokens::Token token) {
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

    std::string print(Expr::Expr* node) {
        return std::get<std::string>(node->accept(this));
    }
};

std::string Printer::print(Expr::Expr *tree) {
    prettyPrinter printer;
    return printer.print(tree);
}
