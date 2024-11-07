#include "evaluator.h"

#include <algorithm>        // std::transform
#include <cmath>            // std::fmod, std::pow
#include <functional>       // std::function
#include <iterator>         // std::back_inserter
#include <optional>         // std::optional

using namespace std::literals;

static expr::function_result call_function(
    const std::string& name,
    const expr::function_t& function,
    const expr::location_t& location,
    const std::vector<expr::node_ptr>& parameters,
    expr::symbol_table& symbols,
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
            if (auto result = expr::evaluate(unevaluated, symbols, functions))
                return *result;
            failed = true;
            return 0.0;
        }
    );
    if (failed) {
        return expr::error{
            .code = expr::error_code::EVALUATOR_FAILED_TO_EVALUATE_ARGUMENTS,
            .location = location,
            .description = "Failed to evaluate function arguments for '"s +
                           name + "()'."
        };
    }
    return function(evaluated, location);
}

static std::optional<double> parse_binary_number(const std::string& content) {
    if (content.substr(0, 2) != "0b")
        return std::nullopt;

    return double(std::strtol(content.substr(2).c_str(), nullptr, 2));
}

static std::optional<double> parse_octal_number(const std::string& content) {
    if (content.length() == 0 || content[0] != '0')
        return std::nullopt;

    auto valid_octal_char = [](char c) { return (c >= '0' && c <= '7'); };
    if (!std::all_of(content.begin(), content.end(), valid_octal_char))
        return std::nullopt;

    return double(std::strtol(content.substr(1).c_str(), nullptr, 8));
}

expr::evaluator_result expr::evaluate(
    const expr::node_ptr& node,
    expr::symbol_table& symbols,
    const expr::function_table& functions
) {
    using binary_operator = std::function<double(double, double)>;
    static const std::unordered_map<std::string, binary_operator> binary = {
        {"+", [](double lhs, double rhs) { return lhs + rhs; }},
        {"-", [](double lhs, double rhs) { return lhs - rhs; }},
        {"*", [](double lhs, double rhs) { return lhs * rhs; }},
        {"/", [](double lhs, double rhs) { return lhs / rhs; }},
        {"%", [](double lhs, double rhs) { return std::fmod(lhs, rhs); }},
        {"^", [](double lhs, double rhs) { return std::pow(lhs, rhs); }},
    };

    using unary_operator = std::function<double(double)>;
    static const std::unordered_map<std::string, unary_operator> unary = {
        {"+", [](double operand) { return operand; }},
        {"-", [](double operand) { return -1 * operand; }},
    };

    switch (node->type) {
        case expr::node_t::type_t::BINARY_OP: {
            const auto left = expr::evaluate(
                node->children[0],
                symbols,
                functions
            );
            const auto right = expr::evaluate(
                node->children[1],
                symbols,
                functions
            );
            if (!left || !right) {
                return expr::error{
                    .code = expr::error_code::EVALUATOR_FAILED_TO_EVALUATE_OPERAND,
                    .location = node->location,
                    .description = "Failed to evaluate operand."
                };
            }
            const auto& f = binary.at(node->content);
            return f(*left, *right);
        }
        case expr::node_t::type_t::UNARY_OP: {
            const auto operand = expr::evaluate(
                node->children[0],
                symbols,
                functions
            );
            if (operand.has_value()) {
                const auto& f = unary.at(node->content);
                return f(*operand);
            }
            return expr::error{
                .code = expr::error_code::EVALUATOR_FAILED_TO_EVALUATE_OPERAND,
                .location = node->location,
                .description = "Failed to evaluate operand."
            };
        }
        case expr::node_t::type_t::NUMBER: {
            if (auto bin = parse_binary_number(node->content))
                return *bin;
            if (auto oct = parse_octal_number(node->content))
                return *oct;
            return std::strtod(node->content.c_str(), nullptr);
        }
        case expr::node_t::type_t::VARIABLE:
            if (symbols.find(node->content) == symbols.end()) {
                return expr::error{
                    .code = expr::error_code::EVALUATOR_UNDEFINED_VARIABLE,
                    .location = node->location,
                    .description = "Undefined variable '" + node->content + "'."
                };
            }
            return symbols.at(node->content);
        case expr::node_t::type_t::FUNCTION_CALL:
            if (functions.find(node->content) == functions.end()) {
                const auto& where = node->location.begin;
                return expr::error{
                    .code = expr::error_code::EVALUATOR_UNDEFINED_FUNCTION,
                    .location = expr::location_t{
                        .begin = where,
                        .end = where + node->content.length() - 1
                    },
                    .description = "Undefined function '"s + node->content + "'."
                };
            }
            return call_function(
                node->content,
                functions.at(node->content).implementation,
                node->location,
                node->children,
                symbols,
                functions
            );
        case expr::node_t::type_t::ASSIGNMENT:
            auto result = expr::evaluate(node->children[1], symbols, functions);
            symbols[node->children[0]->content] = result;
            return result;
    }

    // Unreachable
    return expr::error{
        .code = expr::error_code::EVALUATOR_REACHED_UNREACHABLE_CODE_PATH,
        .location = expr::location_t{.begin = 0, .end = 0},
        .description = "The evaluator has reached a supposedly unreachable "
                       "code path."
    };
}

expr::evaluator_result expr::evaluate_parse_time(const expr::node_ptr& node) {
    expr::symbol_table table;
    return expr::evaluate(node, table, expr::function_table{});
}
