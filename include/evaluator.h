#if !defined(EXPRPARSER_EVALUATOR_HEADER)
#define EXPRPARSER_EVALUATOR_HEADER

#include "node.h"

#include <optional>
#include <unordered_map>

namespace expr {
    using symbol_table = std::unordered_map<std::string, double>;

    std::optional<double> evaluate(
        const node_ptr& node,
        const symbol_table& symbols);
}

#endif
