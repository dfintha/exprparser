#if !defined(EXPRPARSER_FUNCTIONS_HEADER)
#define EXPRPARSER_FUNCTIONS_HEADER

#include "location.h"
#include "result.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace expr {
    using function_result = result<double, error>;

    using function_t = function_result (*)(
        const std::vector<double>&,
        const expr::location_t&
    );

    using function_table = std::unordered_map<std::string, function_t>;

    const function_table& functions();
}

#endif
