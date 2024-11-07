#if !defined(EXPRPARSER_EVALUATOR_HEADER)
#define EXPRPARSER_EVALUATOR_HEADER

#include "functions.h"
#include "node.h"

namespace expr {
    using evaluator_result = function_result;
    using symbol_table = std::unordered_map<std::string, double>;

    evaluator_result evaluate(
        const node_ptr& node,
        symbol_table& symbols,
        const function_table& functions
    );

    evaluator_result evaluate_parse_time(const node_ptr& node);
}

#endif
