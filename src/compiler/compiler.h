#pragma once
#include <string>

#include "../AST/expr.h"

namespace Compiler {
    typedef struct {
        Expr::Expr* expr;
    } CompileResult;

    CompileResult compile(const std::string &source);
}
