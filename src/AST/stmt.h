#pragma once

#include <variant>


namespace Stmt {

    #define StmtVisitResults std::variant<>

    class StmtVisitor {
    public:
        virtual ~StmtVisitor() = default;

    };

    struct Stmt {
        virtual ~Stmt() = default;
        virtual StmtVisitResults accept(StmtVisitor *visitor) = 0;
    };
}