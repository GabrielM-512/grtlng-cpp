#pragma once
#include <string>
#include <utility>
#include <vector>

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

    class Obj;

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

    enum ObjectType {
        CALLABLE
    };

    bool isObjType(const Value& val, ObjectType type);

    class Obj {
    public:
        virtual ~Obj() = default;

        ObjectType type;

        Obj(ObjectType type): type(type) {}
    };

    class Callable : public Obj {
    protected:
        std::string name;
        std::vector<Stmt::VariableDeclaration*> params;
    public:
        Callable(std::string name, std::vector<Stmt::VariableDeclaration*> params): Obj(CALLABLE),
            name(std::move(name)), params(std::move(params)) {}

        [[nodiscard]] std::string getName() const {return name;}

        [[nodiscard]] int getArity() const {return (int) params.size();}
        virtual Value call(Interpreting::Interpreter* interpreter, std::vector<Value>& args) = 0;

    };

    class Function : public Callable {

        Stmt::Block* body;
    public:
        Function(std::string name, std::vector<Stmt::VariableDeclaration*> params, Stmt::Block* body): Callable(std::move(name), std::move(params)),
            body(body) {}

        Value call(Interpreting::Interpreter *interpreter, std::vector<Value> &args) override;

    };


    void defineNativeFunctions(Interpreting::Interpreter* interpreter);
    void defineNativesResolver(Resolving::Scope* scope);

#define VALUE_NUM(number) ((Value::Value) {.type = Value::NUMBER, .as = {.num = (number)}})
#define VALUE_CALLABLE(callable) ((Value::Value) {.type = Value::OBJECT, .as = {.object = callable}})
#define VALUE_TRUE (VALUE_NUM(1))
#define VALUE_FALSE (VALUE_NUM(0))

#define IS_NUM(value) ((value).type == NUMBER)
#define IS_CALLABLE(value) (Value::isObjType((value), Value::CALLABLE))

#define AS_NUM(value) ((value).as.num)
#define AS_CALLABLE(value) ((Value::Callable*)(value).as.object)

    void printValue(const Value& value);
    bool isTruthy(const Value& value);
}
