#include "value.h"
#include "iostream"
#include "compiler/lexing.h"
#include "interpreter/runtimeException.h"

void printObject(const Value::Obj*) {

}

void Value::printValue(const Value& value) {
    switch (value.type) {
        case NUMBER:
            std::cout << AS_NUM(value) << std::endl;
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
