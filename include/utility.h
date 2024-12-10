#if !defined(EXPRPARSER_UTILITY_HEADER)
#define EXPRPARSER_UTILITY_HEADER

#include <cfloat>           // DBL_EPSILON
#include <cmath>            // std::fabs

namespace expr {
    inline bool is_near(double lhs, double rhs) noexcept {
        return std::fabs(lhs - rhs) <= DBL_EPSILON;
    }
}

#endif
