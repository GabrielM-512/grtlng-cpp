#pragma once
#include <string>

#include "../error.h"
#include "../AST/stmt.h"

namespace Compiler {
    struct CompileResult {
        bool success = true;
        std::vector<Stmt::Stmt*> tree;
    };

    CompileResult compile(const std::string &source, Error::ErrorHandler& errorHandler);
}
