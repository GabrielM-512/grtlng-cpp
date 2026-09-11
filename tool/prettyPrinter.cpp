#include "prettyPrinter.h"

#include <format>

#include "../src/AST/expr.h"

class PrettyPrinter : public Expr::ExprVisitor, Stmt::StmtVisitor {
public:

    static std::string operatorString(Lexing::TokenType type) {
        return Lexing::Token::toString(type).substr(1, 1);
    }

    static std::string operatorString(Lexing::Token token) {
        return operatorString(token.type);
    }

    std::string expression(Expr::Expr* expr) {
        return std::get<std::string> (expr->accept(this));
    }

    std::string statement(Stmt::Stmt* stmt) {
        return std::get<std::string> (stmt->accept(this));
    }

    ExprVisitResults visitBinaryExpr(Expr::Binary* expr) override {
        return "( " + expression(expr->left) + " " +
                    operatorString(expr->operatorType) + " " +
                    expression(expr->right) + " )";
    }

    ExprVisitResults visitUnaryExpr(Expr::Unary* expr) override {
        return "( " + operatorString(expr->operatorType) +
            expression (expr->right) + " )";
    }

    ExprVisitResults visitNumberExpr(Expr::Number* expr) override {
        return std::to_string(expr->value);
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier* expr) override {
        return expr->target.data.name;
    }

    ExprVisitResults visitAssignExpr(Expr::Assign *expr) override {
        return "( " + std::string(expr->name.data.name) + " = "
                    + expression(expr->value) + " )";
    }


    StmtVisitResults visitExpressionStmt(Stmt::Expression *stmt) override {
        return "[EXPR] " + expression(stmt->expression);
    }

    StmtVisitResults visitBlockStmt(Stmt::Block *stmt) override {
        std::string output = "{ ";

        for (Stmt::Stmt* current : stmt->statements) {
            output.append(statement(current) + "\n");
        }

        return output;
    }

    StmtVisitResults visitIfStmt(Stmt::If *stmt) override {
        std::string output = "IF (" + expression(stmt->condition) + ")\n"
        + statement(stmt->thenBranch);

        if (stmt->elseBranch == nullptr) output.append("\n NO ELSE");
        else output.append("ELSE\n" + statement(stmt->elseBranch));

        return output;
    }

    StmtVisitResults visitPrintStmt(Stmt::Print *stmt) override {
        return "PRINT " + expression(stmt->expression);
    }

    StmtVisitResults visitWhileStmt(Stmt::While *stmt) override {
        return "WHILE (" + expression(stmt->condition) + ") DO\n"
            + statement(stmt->body);
    }

    StmtVisitResults visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) override {
        return "Declare Variable '}" + std::string(stmt->name.data.name) + "' of type " + Lexing::Token::toString(stmt->dataType) +
            (stmt->value == nullptr ? " without value" : " with value = " + expression(stmt->value));
    }

    std::string print(const std::vector<Stmt::Stmt*>& tree) {
        std::string output;

        for (Stmt::Stmt* current : tree) {
            output.append(statement(current) + "\n");
        }

        return output;
    }
};

std::string Printer::print(std::vector<Stmt::Stmt*> tree) {
    PrettyPrinter printer;
    return printer.print(tree);
}
