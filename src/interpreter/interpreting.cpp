#include "interpreting.h"
#include "environment.h"

using namespace Interpreting;

void Interpreter::beginEnvironment() {
    current = new Environment(current);
}

void Interpreter::endEnvironment() {
    Environment* restore = current->enclosing;
    delete current;
    current = restore;
}

Interpreter::Interpreter() {
    global = Environment();
    current = &global;
}

Interpreter::~Interpreter() {
    while (current->enclosing != nullptr) {
        endEnvironment();
    }
}

Value::Value Interpreter::interpret(Expr::Expr* program) {
    return evaluate(program);
}

Value::Value Interpreting::interpret(Expr::Expr *program) {
    Interpreter interpreter;


    double returnCode = interpreter.interpret(program);

    return returnCode;

}
