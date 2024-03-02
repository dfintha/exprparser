#if !defined(EXPRPARSER_LOCATION_HEADER)
#define EXPRPARSER_LOCATION_HEADER

#include <cstddef>

namespace expr {
    struct location_t {
        size_t begin;
        size_t end;
    };
}

#endif
