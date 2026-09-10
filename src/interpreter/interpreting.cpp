#include "interpreting.h"

#include <sysexits.h>

#include "environment.h"
#include "../error.h"
#include "../compiler/compiler.h"

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

void Interpreter::interpret(Compiler::CompileResult& program) {
    for (Stmt::Stmt* stmt : program.tree) {
        execute(stmt);
    }
}

Value::Value Interpreting::interpret(Compiler::CompileResult& program, Error::ErrorHandler& handler) {
    
    try {
        Interpreter interpreter;
        interpreter.interpret(program);
    } catch (RuntimeException& e) {
        handler.runtimeError(e);
        exit(EX_DATAERR);
    }

    return 0;

}
