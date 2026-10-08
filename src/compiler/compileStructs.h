#pragma once

// structs used in several headers which would introduce circular imports if declared anywhere else

namespace structs {
    struct test {
        bool isConstexpr;
    };
}