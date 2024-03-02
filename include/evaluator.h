#if !defined(EXPRPARSER_EVALUATOR_HEADER)
#define EXPRPARSER_EVALUATOR_HEADER

#include "node.h"
#include "result.h"

#include <optional>
#include <unordered_map>

namespace expr {
    using evaluator_result = result<double, error>;
    using function_t = evaluator_result (*)(
        const std::vector<double>&,
        const expr::location_t&
    );

    using symbol_table = std::unordered_map<std::string, double>;
    using function_table = std::unordered_map<std::string, function_t>;

    evaluator_result evaluate(
        const node_ptr& node,
        const symbol_table& symbols,
        const function_table& functions);
}

#endif
