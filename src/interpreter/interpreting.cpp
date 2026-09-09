#include "interpreting.h"

#include <sysexits.h>

#include "environment.h"
#include "../error.h"

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

Value::Value Interpreting::interpret(Expr::Expr *program, Error::ErrorHandler& handler) {
    Interpreter interpreter;

    double returnCode;

    try {
        returnCode = interpreter.interpret(program);
    } catch (RuntimeException& e) {
        handler.runtimeError(e);
        exit(EX_DATAERR);
    }


    return returnCode;

}
