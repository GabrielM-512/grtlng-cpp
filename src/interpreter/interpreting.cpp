#include "interpreting.h"

#include <iostream>
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

void Interpreter::addGlobalValue(const std::string& name, Value::Value value) {
    global.createVar(name, value);
}


Interpreter::Interpreter() {
    global = Environment();
    current = &global;

    Value::defineNativeFunctions(this);
}

Interpreter::~Interpreter() {
    while (current->enclosing != nullptr) {
        endEnvironment();
    }
}

int Interpreter::interpret(Compiler::CompileResult& program) {
    for (Stmt::Stmt* stmt : program.tree) {
        execute(stmt);
    }

    std::vector<Value::Value> inputs;

    Value::Value returnValue = AS_CALLABLE(global.getVar("main"))->call(this, inputs);
    return (int) AS_NUM(returnValue);
}

f64 Interpreting::interpret(Compiler::CompileResult& program, Error::ErrorHandler& handler) {
    int returnValue;

    try {
        Interpreter interpreter;
        returnValue = interpreter.interpret(program);
    } catch (RuntimeException& e) {
        if (e.token.type != Lexing::ERROR) {
            handler.runtimeError(e);
        } else {
            std::cerr << "Internal error: " << e.message << std::endl;
        }
        exit(EX_DATAERR);
    }

    return returnValue;

}
