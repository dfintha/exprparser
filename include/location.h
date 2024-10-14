#if !defined(EXPRPARSER_LOCATION_HEADER)
#define EXPRPARSER_LOCATION_HEADER

#include <cstddef>          // std::size_t

namespace expr {
    struct location_t {
        std::size_t begin;
        std::size_t end;
    };
}

#endif
