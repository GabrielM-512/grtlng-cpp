#pragma once

#include <string>
#include <unordered_map>

#include "../value.h"

namespace Interpreting {
    class Environment {
        std::unordered_map<std::string, Value::Value> values;

    public:
        Environment* enclosing;

        Environment();
        Environment(Environment *enclosing);

        ~Environment() = default;

        Value::Value getVar(const std::string& name) const;
        void setVar(const std::string& name, Value::Value var);
        void createVar(const std::string& name, Value::Value var);
    };
}
