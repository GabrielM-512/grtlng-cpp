#include "interpreting.h"

using namespace Interpreting;

StmtVisitResults Interpreter::visitPrintStmt(Stmt::Print *stmt) {
    Value::Value val = evaluate(stmt->expression);
    Value::printValue(val);
    return std::monostate();
}
StmtVisitResults Interpreter::visitExpressionStmt(Stmt::Expression *stmt) {
    evaluate(stmt->expression);
    return std::monostate();
}

StmtVisitResults Interpreter::visitVariableDeclarationStmt(Stmt::VariableDeclaration *stmt) {
    Value::Value val = 0;

    if (stmt->value != nullptr) val = evaluate(stmt->value);

    current->createVar(stmt->name, val);
    return std::monostate();
}

StmtVisitResults Interpreter::visitIfStmt(Stmt::If *stmt) {
    if (Value::isTruthy(evaluate(stmt->condition)))
        execute(stmt->thenBranch);
    else if (stmt->elseBranch != nullptr)
        execute(stmt->elseBranch);

    return std::monostate();
}

StmtVisitResults Interpreter::visitWhileStmt(Stmt::While *stmt) {
    while (Value::isTruthy(evaluate(stmt->condition))) {
        execute(stmt->body);
    }

    return std::monostate();
}

StmtVisitResults Interpreter::visitBlockStmt(Stmt::Block *stmt) {
    beginEnvironment();

    for (Stmt::Stmt* currentStmt : stmt->statements) {
        execute(currentStmt);
    }

    return std::monostate();
}


void Interpreter::execute(Stmt::Stmt* stmt) {
    stmt->accept(this);
}