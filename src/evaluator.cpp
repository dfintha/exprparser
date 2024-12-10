#include "evaluator.h"
#include "utility.h"

#include <algorithm>        // std::transform
#include <cmath>            // std::fmod, std::pow
#include <functional>       // std::function
#include <iterator>         // std::back_inserter
#include <optional>         // std::optional

static expr::evaluator_result evaluate_binary_operator(
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

    const auto left = expr::evaluate(node->children[0], symbols, functions);
    const auto right = expr::evaluate(node->children[1], symbols, functions);
    if (!left || !right) {
        return expr::error{
            .code = expr::error_code::EVALUATOR_FAILED_TO_EVALUATE_OPERAND,
            .location = node->location,
            .description = "Failed to evaluate operand."
        };
    }

    if (node->content == "/" && expr::is_near(*right, 0)) {
        return expr::error{
            .code = expr::error_code::EVALUATOR_DIVISION_BY_ZERO,
            .location = node->children[1]->location,
            .description = "Division by zero."
        };
    }

    const auto& operator_fn = binary.at(node->content);
    return operator_fn(*left, *right);
}

static expr::evaluator_result evaluate_unary_operator(
    const expr::node_ptr& node,
    expr::symbol_table& symbols,
    const expr::function_table& functions
) {
    using unary_operator = std::function<double(double)>;
    static const std::unordered_map<std::string, unary_operator> unary = {
        {"+", [](double operand) { return operand; }},
        {"-", [](double operand) { return -1 * operand; }},
    };

    const auto operand = expr::evaluate(node->children[0], symbols, functions);
    if (operand.has_value()) {
        const auto& operator_fn = unary.at(node->content);
        return operator_fn(*operand);
    }

    return expr::error{
        .code = expr::error_code::EVALUATOR_FAILED_TO_EVALUATE_OPERAND,
        .location = node->location,
        .description = "Failed to evaluate operand."
    };
}

static expr::evaluator_result evaluate_number_literal(
    const expr::node_ptr& node
) {
    if (node->content.substr(0, 2) == "0b")
        return double(std::strtol(node->content.substr(2).c_str(), nullptr, 2));

    if (node->content.length() == 0 || node->content[0] != '0')
        return std::strtod(node->content.c_str(), nullptr);

    auto octal_char = [](char c) { return (c >= '0' && c <= '7'); };
    if (!std::all_of(node->content.begin(), node->content.end(), octal_char)) {
        return expr::error{
            .code = expr::EVALUATOR_INVALID_NUMBER_LITERAL,
            .location = node->location,
            .description = "Invalid numeric literal '" + node->content + "'."
        };
    }

    return double(std::strtol(node->content.substr(1).c_str(), nullptr, 8));
}

static expr::evaluator_result evaluate_variable_reference(
    const expr::node_ptr& node,
    const expr::symbol_table& symbols
) {
    if (symbols.find(node->content) == symbols.end()) {
        return expr::error{
            .code = expr::error_code::EVALUATOR_UNDEFINED_VARIABLE,
            .location = node->location,
            .description = "Undefined variable '" + node->content + "'."
        };
    }
    return symbols.at(node->content);
}

static expr::evaluator_result evaluate_function_call(
    const expr::node_ptr& node,
    expr::symbol_table& symbols,
    const expr::function_table& functions
) {
    if (functions.find(node->content) == functions.end()) {
        const auto& where = node->location.begin;
        return expr::error{
            .code = expr::error_code::EVALUATOR_UNDEFINED_FUNCTION,
            .location = expr::location_t{
                .begin = where,
                .end = where + node->content.length() - 1
            },
            .description = "Undefined function '" + node->content + "'."
        };
    }

    const auto& implementation = functions.at(node->content).implementation;

    bool failed = false;
    std::vector<double> evaluated;
    std::transform(
        node->children.begin(),
        node->children.end(),
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
            .location = node->location,
            .description = "Failed to evaluate function arguments for '" +
                           node->content + "()'."
        };
    }

    return implementation(evaluated, node->location);
}

static expr::evaluator_result evaluate_assignment(
    const expr::node_ptr& node,
    expr::symbol_table& symbols,
    const expr::function_table& functions
) {
    auto result = expr::evaluate(node->children[1], symbols, functions);
    symbols[node->children[0]->content] = result;
    return result;
}

expr::evaluator_result expr::evaluate(
    const expr::node_ptr& node,
    expr::symbol_table& symbols,
    const expr::function_table& functions
) {
    switch (node->type) {
        case expr::node_t::type_t::BINARY_OP:
            return evaluate_binary_operator(node, symbols, functions);
        case expr::node_t::type_t::UNARY_OP:
            return evaluate_unary_operator(node, symbols, functions);
        case expr::node_t::type_t::NUMBER:
            return evaluate_number_literal(node);
        case expr::node_t::type_t::VARIABLE:
            return evaluate_variable_reference(node, symbols);
        case expr::node_t::type_t::FUNCTION_CALL:
            return evaluate_function_call(node, symbols, functions);
        case expr::node_t::type_t::ASSIGNMENT:
            return evaluate_assignment(node, symbols, functions);
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
