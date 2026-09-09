#pragma once
#include <string>

#include "../AST/expr.h"
#include "../error.h"

namespace Compiler {
    typedef struct {
        bool success;
        Expr::Expr* expr;
    } CompileResult;

    CompileResult compile(const std::string &source, Error::ErrorHandler& errorHandler);
}
