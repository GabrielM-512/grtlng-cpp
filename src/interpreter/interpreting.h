#pragma once
#include "../AST/expr.h"

namespace Interpreting {

    class RuntimeException : public std::runtime_error {
    public:
        RuntimeException(const std::string& message) : std::runtime_error(message) {}
    };

    Value::Value interpret(Expr::Expr *program);
}
