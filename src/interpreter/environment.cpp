#include "environment.h"
#include "runtimeException.h"

using namespace Interpreting;

Environment::Environment() : enclosing(nullptr) {
    values = std::unordered_map<std::string, Value::Value>();
}

Environment::Environment(Environment *enclosing): enclosing(enclosing) {
    values = std::unordered_map<std::string, Value::Value>();
}


Value::Value Environment::getVar(const Lexing::Tokens::Token &token) const {
    auto value = values.find(token.data.name);
    if (value == values.end()) {
        if (enclosing == nullptr) throw RuntimeException("Unknown variable \"" + std::string(token.data.name) + "\"", token);
        return enclosing->getVar(token);
    }

    return value->second;
}

void Environment::setVar(const Lexing::Tokens::Token &token, const Value::Value var) {
    auto value = values.find(token.data.name);
    if (value == values.end()) {
        if (enclosing == nullptr) throw RuntimeException("Unknown variable \"" + std::string(token.data.name) + "\"", token);
        enclosing->setVar(token, var);
    }

    value->second = var;
}

void Environment::createVar(const Lexing::Tokens::Token &token, const Value::Value var) {
    auto value = values.find(token.data.name);

    if (value != values.end()) throw RuntimeException("Redeclared variable \"" + std::string(token.data.name) + "\"", token);

    values.insert({token.data.name, var});
}