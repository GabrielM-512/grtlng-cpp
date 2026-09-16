#include "resolving.h"

#include <cstring>
#include <unordered_map>
#include <utility>

using namespace Resolving;

class Resolver : public Expr::ExprVisitor, public Stmt::StmtVisitor {
    Compiler::CompileResult& tree;
    Error::ErrorHandler &handler;
    Scope* current;
    bool globalPhase = true;

    void expression(Expr::Expr* expr) {
        expr->accept(this);
    }

    void statement(Stmt::Stmt* stmt) {
        stmt->accept(this);
    }

    Scope* findScope(const std::string& name) {
        Scope* searching = current;

        while (true) {
            if (searching->hasVar(name)) return searching;

            if (searching->enclosing == nullptr) return nullptr;

            searching = searching->enclosing;
        }
    }

    void beginScope() {
        current = new Scope(current);
    }

    void endScope() {
        Scope* saved = current->enclosing;
        delete current;
        current = saved;
    }

    bool varExists(const std::string& name) {
        return findScope(name) != nullptr;
    }

    bool varActivated(const std::string& name) {
        return findScope(name)->isActivated(name);
    }

    void activateVar(const std::string& name) {
        Scope* location = findScope(name);
        location->activateVar(name);
    }

    void error(std::string message, Lexing::Token token) {
        handler.compileError(std::move(message), "", token);
        tree.success = false;
    }

public:
    Resolver(Compiler::CompileResult& tree, Error::ErrorHandler &handler) : tree(tree), handler(handler) {
        current = new Scope();
        Value::defineNativesResolver(current);
    }

/*
    EEEEE   X   X   PPPP    RRRR    EEEEE    SSSS    SSSS   IIIII    OOO    N   N    SSSS
    E        X X    P   P   R   R   E       S       S        III    O   O   NN  N   S
    EEEEE     X     PPPP    RRRR    EEEEE    SSS     SSS     III    O   O   N N N    SSS
    E        X X    P       R  R    E           S       S    III    O   O   N  NN       S
    EEEEE   X   X   P       R   R   EEEEE   SSSS    SSSS    IIIII    OOO    N   N   SSSS
*/

    ExprVisitResults visitAssignExpr(Expr::Assign *expr) override {
        Expr::Identifier target(expr->name);
        expression(&target);

        expression(expr->value);

        return std::monostate();
    }

    ExprVisitResults visitBinaryExpr(Expr::Binary *expr) override {
        expression(expr->left);
        expression(expr->right);

        return std::monostate();
    }

    ExprVisitResults visitCallExpr(Expr::Call *expr) override {
        if (globalPhase) {
            error("Tried using a function call to initialise a global variable", expr->paren);
        }
        expression(expr->callee);

        for (Expr::Expr* arg : expr->args)
            expression(arg);

        return std::monostate();
    }

    ExprVisitResults visitIdentifierExpr(Expr::Identifier *expr) override {
        if (!varExists(expr->target.data.name)) {
            error("Unknown symbol \"" + std::string(expr->target.data.name) + "\"", expr->target);
            return std::monostate();
        }

        if (!varActivated(expr->target.data.name))
            error("Symbol \"" + std::string(expr->target.data.name) + "\" used in its own initialiser", expr->target);

        return std::monostate();
    }

    ExprVisitResults visitNumberExpr(Expr::Number *) override {
        return std::monostate();
    }

    ExprVisitResults visitUnaryExpr(Expr::Unary *expr) override {
        expression(expr->right);
        return std::monostate();
    }

/*
     SSSS   TTTTT     A     TTTTT   EEEEE   M   M   EEEEE   N   N   TTTTT    SSSS
    S         T      A A      T     E       MM MM   E       NN  N     T     S
     SSS      T      AAA      T     EEEEE   M M M   EEEEE   N N N     T      SSS
        S     T     A   A     T     E       M   M   E       N  NN     T         S
    SSSS      T     A   A     T     EEEEE   M   M   EEEEE   N   N     T     SSSS
*/
    void resolveBlock(Stmt::Block* stmt, Scope* base) {
        if (base == nullptr) current = new Scope(current);
        else current = new Scope(current, *base);

        for (Stmt::Stmt* currentStatement : stmt->statements) {
            statement(currentStatement);
        }

        endScope();
    }

    void resolveBlock(Stmt::Block* stmt) {
        resolveBlock(stmt, nullptr);
    }

    StmtVisitResults visitBlockStmt(Stmt::Block *stmt) override {
        resolveBlock(stmt);
        return std::monostate();
    }

    StmtVisitResults visitExpressionStmt(Stmt::Expression *stmt) override {
        expression(stmt->expression);
        return std::monostate();
    }

    StmtVisitResults visitFunctionStmt(Stmt::Function *stmt) override {
        Scope params;

        for (Stmt::VariableDeclaration* param : stmt->params) {
            const char* name = param->name.data.name;
            if (params.hasVar(name)) {
                error("Redeclared parameter \"" + std::string(param->name.data.name) + "\" in function \"" + std::string(stmt->name.data.name) + "\"", param->name);
                continue;
            }

            if (strlen(name) > 0) {
                params.createVar(name);
                params.activateVar(name);
            }

        }

        globalPhase = false;

        resolveBlock(stmt->body, &params);

        globalPhase = true;

        return std::monostate();
    }

    StmtVisitResults visitIfStmt(Stmt::If *stmt) override {
        expression(stmt->condition);
        statement(stmt->thenBranch);
        if (stmt->elseBranch != nullptr) statement(stmt->elseBranch);

        return std::monostate();
    }

    StmtVisitResults visitPrintStmt(Stmt::Print *stmt) override {
        expression(stmt->expression);
        return std::monostate();
    }

    StmtVisitResults visitReturnStmt(Stmt::Return *stmt) override {
        if (stmt->value != nullptr) expression(stmt->value);
        return std::monostate();
    }

    StmtVisitResults visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) override {
        if (current->hasVar(stmt->name.data.name))
            error("Redeclared variable \"" + std::string(stmt->name.data.name) + "\" in same scope", stmt->name);
        else
            current->createVar(stmt->name.data.name);

        if (stmt->value != nullptr) expression(stmt->value);

        activateVar(stmt->name.data.name);

        return std::monostate();
    }

    StmtVisitResults visitWhileStmt(Stmt::While *stmt) override {
        expression(stmt->condition);
        statement(stmt->body);

        return std::monostate();
    }

    void resolve() {
        std::vector<Stmt::Function*> functions;
        for (Stmt::Stmt* stmt : tree.tree) {
            std::string name;
            if (auto func = dynamic_cast<Stmt::Function*> (stmt)) {
                current->createVar(func->name.data.name);
                activateVar(func->name.data.name);
                functions.push_back(func);
            } else if (auto var = dynamic_cast<Stmt::VariableDeclaration*> (stmt)) {
                statement(var);
            }
        }

        for (Stmt::Function* func : functions) {
            statement(func);
        }
    }
};

void Resolving::resolve(Compiler::CompileResult &program, Error::ErrorHandler &handler) {
    Resolver resolver(program, handler);

    resolver.resolve();
}