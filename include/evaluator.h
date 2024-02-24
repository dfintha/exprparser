#if !defined(EXPRPARSER_EVALUATOR_HEADER)
#define EXPRPARSER_EVALUATOR_HEADER

#include "node.h"

#include <optional>
#include <unordered_map>

namespace expr {
    using function_t = std::optional<double> (*)(const std::vector<double>&);
    using symbol_table = std::unordered_map<std::string, double>;
    using function_table = std::unordered_map<std::string, function_t>;

    std::optional<double> evaluate(
        const node_ptr& node,
        const symbol_table& symbols,
        const function_table& functions);
}

#endif
