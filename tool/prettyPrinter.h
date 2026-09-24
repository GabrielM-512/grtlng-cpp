#pragma once
#include "../src/AST/stmt.h"

namespace Printer {
    std::string print(const std::vector<Stmt::Stmt*>& tree);
}
