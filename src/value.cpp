#include "value.h"
#include "iostream"

void Value::printValue(const Value& value) {
    std::cout << value << std::endl;
}


bool Value::isTruthy(const Value& value) {
    return value != 0;
}