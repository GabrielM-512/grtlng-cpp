#pragma once

#include <string>
#include <unordered_map>
#include "../compiler/lexing.h"


#include "../value.h"

namespace Interpreting {
    class Environment {
        std::unordered_map<std::string, Value::Value> values;

    public:
        Environment* enclosing;

        Environment();
        Environment(Environment *enclosing);

        ~Environment() = default;

        Value::Value getVar(const Lexing::Token &token) const;
        void setVar(const Lexing::Token &token, Value::Value var);
        void createVar(const Lexing::Token &token, Value::Value var);
    };
}
