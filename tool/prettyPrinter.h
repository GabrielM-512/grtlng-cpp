#pragma once
#include "../src/AST/stmt.h"

namespace Printer {
    std::string print(std::vector<Stmt::Stmt*> tree);
}
