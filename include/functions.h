#if !defined(EXPRPARSER_FUNCTIONS_HEADER)
#define EXPRPARSER_FUNCTIONS_HEADER

#include "location.h"
#include "quantity.h"
#include "result.h"

#include <unordered_map>    // std::unordered_map
#include <vector>           // std::vector

namespace expr {
    using function_result = result<quantity, error>;

    using function_t = function_result (*)(
        const std::vector<quantity>&,
        const location_t&
    );

    struct function_definition_t {
        function_t implementation;
        std::string signature;
    };

    using function_table =
        std::unordered_map<std::string, function_definition_t>;

    const function_table& functions();
}

#endif
