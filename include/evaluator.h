#if !defined(EXPRPARSER_EVALUATOR_HEADER)
#define EXPRPARSER_EVALUATOR_HEADER

#include "functions.h"
#include "node.h"

#include <string>
#include <unordered_map>

namespace expr {
    using evaluator_result = function_result;
    using symbol_table = std::unordered_map<std::string, double>;

    evaluator_result evaluate(
        const node_ptr& node,
        const symbol_table& symbols,
        const function_table& functions
    );

    std::string to_expression_string(const node_ptr& root);
}

#endif
