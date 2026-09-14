#include "environment.h"
#include "runtimeException.h"

using namespace Interpreting;

Environment::Environment() : enclosing(nullptr) {
    values = std::unordered_map<std::string, Value::Value>();
}

Environment::Environment(Environment *enclosing): enclosing(enclosing) {
    values = std::unordered_map<std::string, Value::Value>();
}


Value::Value Environment::getVar(const std::string &name) const {
    auto value = values.find(name);
    if (value == values.end()) {
        if (enclosing == nullptr) throw RuntimeException("Unknown variable \"" + std::string(name) + "\"");
        return enclosing->getVar(name);
    }

    return value->second;
}

void Environment::setVar(const std::string &name, const Value::Value var) {
    auto value = values.find(name);

    if (value != values.end()) {
        value->second = var;
        return;
    }

    if (enclosing == nullptr) throw RuntimeException("Unknown variable \"" + std::string(name) + "\"");
    enclosing->setVar(name, var);
}

void Environment::createVar(const std::string& name, const Value::Value var) {
    auto value = values.find(name);

    if (value != values.end()) throw RuntimeException("Redeclared variable \"" + std::string(name) + "\"");

    values.insert({name, var});
}