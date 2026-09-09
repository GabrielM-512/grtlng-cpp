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

void Interpreter::execute(Stmt::Stmt* stmt) {
    stmt->accept(this);
}