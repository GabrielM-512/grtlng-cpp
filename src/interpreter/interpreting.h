#pragma once
#include "../AST/expr.h"

namespace Interpreting {
    Value::Value interpret(Expr::Expr *program);
}
