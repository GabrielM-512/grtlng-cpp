#include "environment.h"
#include "interpreting.h"

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
        if (enclosing == nullptr) throw RuntimeException("Unknown variable \"" + name + "\"");
        return enclosing->getVar(name);
    }

    return value->second;
}

void Environment::setVar(const std::string& name, const Value::Value var) {
    auto value = values.find(name);
    if (value == values.end()) {
        if (enclosing == nullptr) throw RuntimeException("Unknown variable \"" + name + "\"");
        enclosing->setVar(name, var);
    }

    value->second = var;
}

void Environment::createVar(const std::string& name, const Value::Value var) {
    auto value = values.find(name);

    if (value != values.end()) throw RuntimeException("Redeclared variable \"" + name + "\"");

    values.insert({name, var});
}