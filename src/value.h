#pragma once

namespace Value {

    enum ValueType {
        NUMBER,
        OBJECT
    };

    class Obj {

    };

    typedef struct {
        ValueType type;
        union {
            double num;
            Obj* object;
        } as;
    } Value;

#define VALUE_NUM(number) ((Value::Value) {.type = Value::NUMBER, .as = {.num = number}})

#define IS_NUM(value) ((value).type == NUMBER)

#define AS_NUM(value) ((value).as.num)

    void printValue(const Value& value);
    bool isTruthy(const Value& value);
}