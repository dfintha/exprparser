#if !defined(EXPRPARSER_LOCATION_HEADER)
#define EXPRPARSER_LOCATION_HEADER

#include <cstddef>          // std::size_t
#include <iosfwd>           // std::ostream

namespace expr {
    struct location_t {
        std::size_t begin;
        std::size_t end;

        bool operator==(const location_t&) const = default;
    };
}

std::ostream& operator<<(std::ostream& stream, const expr::location_t& list);

#endif
