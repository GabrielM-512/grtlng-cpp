#pragma once
#include <string>
#include <utility>
#include <vector>

#include "global.h"

// forward declarations due to header madness.

/*
 * WE MAY NEVER INCLUDE "interpreting.h" OR "AST/stmt.h"!!!
 */

namespace Interpreting {
    class Interpreter;
}

namespace Stmt {
    struct VariableDeclaration;
    struct Block;
}

namespace Resolving {
    class Scope;
}

namespace Value {

    enum ObjectType {
        FUNCTION,
        NATIVE_FN
    };

    class Obj {
    public:
        virtual ~Obj() = default;

        ObjectType type;

        Obj(ObjectType type): type(type) {}
    };

    enum ValueType {
        NUMBER,
        OBJECT
    };

    struct Value {
        ValueType type;
        union {
            double num;
            Obj* object;
        } as;
    };

    struct Function : Obj {
        std::vector<u8> code;
        std::string name;
        u8 arity;

        Function(const std::vector<u8> &code, const std::string &name, u8 arity): Obj(FUNCTION), code(code), name(name), arity(arity) {}
    };

    typedef Value (*NativeFunction)(Value*);

    struct NativeFn : Obj {
        NativeFunction fn;
        std::string name;
        u8 arity;

        NativeFn(NativeFunction func, std::string  name, u8 arity): Obj(NATIVE_FN), fn(func), name(std::move(name)), arity(arity) {}
    };

    bool isObjType(const Value& val, ObjectType type);
    bool equality(Value a, Value b);

    void defineNativesResolver(Resolving::Scope* scope);
    std::vector<NativeFn> nativeFnDefinitions();

#define VALUE_FUNCTION(func) ((Value::Value) {.type = Value::OBJECT, .as = {.object = func}})
#define VALUE_NATIVE(native) ((Value::Value) {.type = Value::OBJECT, .as = {.object = native}})
#define VALUE_NUM(number) ((Value::Value) {.type = Value::NUMBER, .as = {.num = static_cast<double>(number)}})
#define VALUE_TRUE (VALUE_NUM(1))
#define VALUE_FALSE (VALUE_NUM(0))
#define VALUE_BOOL(boolean) ((boolean) ? VALUE_TRUE : VALUE_FALSE)

#define IS_FUNC(value) (Value::isObjType((value), Value::FUNCTION))
#define IS_NATIVE(value) (Value::isObjType((value), Value::NATIVE_FN))
#define IS_NUM(value) ((value).type == NUMBER)
#define IS_OBJ(value) ((value).type == Value::OBJECT)

#define AS_FUNCTION(value) ((Value::Function*)(value).as.object)
#define AS_NATIVE(value) ((Value::NativeFn*)(value).as.object)
#define AS_NUM(value) ((value).as.num)
#define AS_OBJ(value) ((value).as.object)

    void printValue(const Value& value);
    std::string getValueString(const Value& value);
    bool isTruthy(const Value& value);
}
