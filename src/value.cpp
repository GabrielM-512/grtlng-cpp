#include "value.h"

#include <cstring>
#include <utility>

#include "iostream"
#include "compiler/lexing.h"
#include "interpreter/runtimeException.h"
#include "interpreter/interpreting.h"

/*
    N   N     A     TTTTT   IIIII   V   V   EEEEE           FFFFF   U   U   N   N     CCC   TTTTT   IIIII    OOO    N   N    SSSS
    NN  N    A A      T       I     V   V   E               F       U   U   NN  N    C        T       I     O   O   NN  N   S
    N N N    AAA      T       I      V V    EEEEE           FFFFF   U   U   N N N   C         T       I     O   O   N N N    SSS
    N  NN   A   A     T       I      V V    E               F       U   U   N  NN    C        T       I     O   O   N  NN       S
    N   N   A   A     T     IIIII     V     EEEEE           F        UUU    N   N     CCC     T     IIIII    OOO    N   N   SSSS
*/

typedef Value::Value (*NativeFunction)(const std::vector<Value::Value>&);

Value::Value clockNative(const std::vector<Value::Value>&) {
    return VALUE_NUM((double) clock() / CLOCKS_PER_SEC);
}

class NativeFn : public Value::Callable {
    NativeFunction function;
public:
    NativeFn(std::string name, std::vector<Stmt::VariableDeclaration*> params, NativeFunction function):
        Callable(std::move(name), std::move(params)), function(function) {}

    Value::Value call(Interpreting::Interpreter *, std::vector<Value::Value> &args) override {
        return function(args);
    }
};

void defineNativeFn(Interpreting::Interpreter* interpreter, const std::string& name, std::vector<Stmt::VariableDeclaration*> params, NativeFunction fn) {
    Value::Obj* function = new NativeFn(name, std::move(params), fn);

    interpreter->addGlobalValue(name, VALUE_CALLABLE(function));
}

void defineNativeFn(Interpreting::Interpreter* interpreter, const std::string& name, NativeFunction fn) {
    defineNativeFn(interpreter, name, std::vector<Stmt::VariableDeclaration*>(), fn);
}

void Value::defineNativeFunctions(Interpreting::Interpreter* interpreter) {
    defineNativeFn(interpreter, "clock", clockNative);
}

Value::Value Value::Function::call(Interpreting::Interpreter *interpreter, std::vector<Value> &args) {
    if (args.size() != params.size()) {
        throw Interpreting::RuntimeException(std::format("Function \"{:s}\" expected {} arguments, got {} instead", name, params.size(), args.size()));
    }

    Interpreting::Environment environment(&interpreter->global);

    for (u64 i = 0; i < params.size(); i++) {
        if (params.at(i)->name.data.name[0] != '\0')
            environment.createVar(params.at(i)->name.data.name, args.at(i));
    }

    try {
        interpreter->executeBlock(this->body, &environment);
    } catch (Interpreting::ReturnException& e) {
        return e.value;
    }

    return (Value) {.type = NUMBER, .as = {}};
}

/*
     OOO    BBBB        J   EEEEE     CCC   TTTTT           H   H     A     N   N   DDDD    L       IIIII   N   N    GGG
    O   O   B   B       J   E        C        T             H   H    A A    NN  N   D   D   L         I     NN  N   G
    O   O   BBBB        J   EEEEE   C         T             HHHHH    AAA    N N N   D   D   L         I     N N N   G  GG
    O   O   B   B   J   J   E        C        T             H   H   A   A   N  NN   D   D   L         I     N  NN   G   G
     OOO    BBBB     JJJ    EEEEE     CCC     T             H   H   A   A   N   N   DDDD    LLLLL   IIIII   N   N    GGG
*/

bool Value::isObjType(const Value& val, ObjectType type) {
    return val.type == OBJECT && val.as.object->type == type;
}

void printObject(const Value::Obj* obj) {
    switch (obj->type) {
        case Value::CALLABLE: {
            auto callable = dynamic_cast<const Value::Callable*> (obj);
            std::cout << "<fn \"" << callable->getName() << "\" at " << obj << ">\n";
        }

    }
}

void Value::printValue(const Value& value) {
    switch (value.type) {
        case NUMBER:
            std::cout << AS_NUM(value) << "\n";
            break;
        case OBJECT:
            printObject(value.as.object);
            break;
    }
}


bool Value::isTruthy(const Value& value) {
    switch (value.type) {
        case NUMBER:
            return AS_NUM(value) != 0;
        case OBJECT:
            return true;
    }
    throw Interpreting::RuntimeException("Invalid value type", (Lexing::Token) {.type = Lexing::ERROR, .line = 0, .position = 0, .data = {.number = 0}});
}
