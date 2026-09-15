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
    Value::Value val = VALUE_NUM(0);

    if (stmt->value != nullptr) val = evaluate(stmt->value);

    current->createVar(stmt->name.data.name, val);
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

StmtVisitResults Interpreter::executeBlock(Stmt::Block *stmt, Environment *environment) {
    Environment* savedEnvironment = current;

    current = environment;

    for (Stmt::Stmt* currentStmt : stmt->statements) {
        try {
            execute(currentStmt);
        } catch (std::runtime_error&) {
            current = savedEnvironment;
            throw;
        }
    }

    current = savedEnvironment;

    return std::monostate();

}

StmtVisitResults Interpreter::visitBlockStmt(Stmt::Block *stmt) {
    beginEnvironment();

    for (Stmt::Stmt* currentStmt : stmt->statements) {
        execute(currentStmt);
    }

    endEnvironment();

    return std::monostate();
}

StmtVisitResults Interpreter::visitFunctionStmt(Stmt::Function *stmt) {
    //std::string name, std::vector<Stmt::VariableDeclaration*> params, Stmt::Block* body
    Value::Function* function = new Value::Function(stmt->name.data.name, stmt->params, stmt->body);

    global.createVar(function->getName(), VALUE_CALLABLE(function));

    return std::monostate();
}

StmtVisitResults Interpreter::visitReturnStmt(Stmt::Return *stmt) {
    Value::Value returnValue = evaluate(stmt->value);

    throw ReturnException(returnValue);
}


void Interpreter::execute(Stmt::Stmt* stmt) {
    stmt->accept(this);
}