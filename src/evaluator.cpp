#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <functional>

static expr::evaluator_result call_function(
    const std::string& name,
    expr::function_t function,
    expr::location_t location,
    const std::vector<expr::node_ptr>& parameters,
    const expr::symbol_table& symbols,
    const expr::function_table& functions
) {
    bool failed = false;
    std::vector<double> evaluated;
    std::transform(
        parameters.begin(),
        parameters.end(),
        std::back_inserter(evaluated),
        [&] (const expr::node_ptr& unevaluated) {
            if (failed)
                return 0.0;
            if (auto result = evaluate(unevaluated, symbols, functions))
                return *result;
            failed = true;
            return 0.0;
        }
    );
    if (failed) {
        return expr::error{
            expr::error_code::EVALUATOR_FAILED_TO_EVALUATE_ARGUMENTS,
            location,
            std::string("failed to evaluate function arguments for '")
                + name
                + "()'"
        };
    }
    return function(evaluated, location);
}

namespace expr {
    evaluator_result evaluate(
        const node_ptr& node,
        const symbol_table& symbols,
        const function_table& functions
    ) {
        using binary_operator = std::function<double(double, double)>;
        static const std::unordered_map<std::string, binary_operator> binary = {
            {"+", [](double lhs, double rhs) { return lhs + rhs; }},
            {"-", [](double lhs, double rhs) { return lhs - rhs; }},
            {"*", [](double lhs, double rhs) { return lhs * rhs; }},
            {"/", [](double lhs, double rhs) { return lhs / rhs; }},
            {"^", [](double lhs, double rhs) { return pow(lhs, rhs); }},
        };

        using unary_operator = std::function<double(double)>;
        static const std::unordered_map<std::string, unary_operator> unary = {
            {"+", [](double operand) { return operand; }},
            {"-", [](double operand) { return -1 * operand; }},
        };

        switch (node->type) {
            case node_t::type_t::BINARY_OP: {
                const auto left = evaluate(
                    node->children[0],
                    symbols,
                    functions
                );
                const auto right = evaluate(
                    node->children[1],
                    symbols,
                    functions
                );
                if (!left || !right) {
                    return error{
                        error_code::EVALUATOR_FAILED_TO_EVALUATE_OPERAND,
                        node->location,
                        "failed to evaluate operand"
                    };
                }
                const auto& f = binary.at(node->content);
                return f(*left, *right);
            }
            case node_t::type_t::UNARY_OP: {
                const auto operand = evaluate(
                    node->children[0],
                    symbols,
                    functions
                );
                if (operand.has_value()) {
                    const auto& f = unary.at(node->content);
                    return f(*operand);
                }
                return error{
                    error_code::EVALUATOR_FAILED_TO_EVALUATE_OPERAND,
                    node->location,
                    "failed to evaluate operand"
                };
            }
            case node_t::type_t::NUMBER: {
                const auto result = strtod(node->content.c_str(), nullptr);
                return result;
            }
            case node_t::type_t::BOOLEAN:
                return node->content == "true" ? 1.0 : 0.0;
            case node_t::type_t::VARIABLE:
                if (symbols.find(node->content) == symbols.end()) {
                    return error{
                        error_code::EVALUATOR_UNDEFINED_VARIABLE,
                        node->location,
                        std::string("undefined variable '") + node->content + "'"
                    };
                }
                return symbols.at(node->content);
            case node_t::type_t::FUNCTION_CALL:
                if (functions.find(node->content) == functions.end()) {
                    return error{
                        error_code::EVALUATOR_UNDEFINED_FUNCTION,
                        node->location,
                        std::string("undefined function '") + node->content + "'"
                    };
                }
                return call_function(
                    node->content,
                    functions.at(node->content),
                    node->location,
                    node->children,
                    symbols,
                    functions
                );
        }
    }
}
