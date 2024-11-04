#include "evaluator.h"

#include <algorithm>        // std::transform
#include <cmath>            // std::fmod, std::pow
#include <functional>       // std::function
#include <iterator>         // std::back_inserter
#include <optional>         // std::optional

using namespace std::literals;

static expr::function_result call_function(
    const std::string& name,
    expr::function_t function,
    expr::location_t location,
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

static int get_precedence_score(const expr::node_ptr& node) {
    switch (node->type) {
        case expr::node_t::type_t::ASSIGNMENT:
            return 0;
        case expr::node_t::type_t::BINARY_OP: {
            if (node->content == "+" || node->content == "-")
                return 1;
            if (node->content == "*" || node->content == "/")
                return 2;
            return 3;
        }
        case expr::node_t::type_t::FUNCTION_CALL:
            return 4;
        case expr::node_t::type_t::UNARY_OP:
            return 5;
        case expr::node_t::type_t::NUMBER:
        case expr::node_t::type_t::VARIABLE:
            return 6;
    }

    // Unreachable
    return 0;
}

static std::string binary_op_to_expression_string(const expr::node_ptr& node) {
    const auto left_precedence = get_precedence_score(node->children[0]);
    const auto right_precedence = get_precedence_score(node->children[1]);
    const auto parent_precedence = get_precedence_score(node);
    const bool left_lesser = left_precedence < parent_precedence;
    const bool right_lesser = right_precedence < parent_precedence;
    return (left_lesser ? "(" : "")
        + expr::to_expression_string(node->children[0])
        + (left_lesser ? ")" : "")
        + " " + node->content + " "
        + (right_lesser ? "(" : "")
        + expr::to_expression_string(node->children[1])
        + (right_lesser ? ")" : "");
}

static std::string unary_op_to_expression_string(const expr::node_ptr& node) {
    const auto& type = node->children[0]->type;
    const bool child_number = type == expr::node_t::type_t::NUMBER;
    const bool child_variable = type == expr::node_t::type_t::VARIABLE;
    const bool child_primary = child_number || child_variable;
    return node->content
        + (!child_primary ? "(" : "")
        + expr::to_expression_string(node->children[0])
        + (!child_primary ? ")" : "");
}

static std::string function_call_to_expression_string(
    const expr::node_ptr& node
) {
    std::string result = node->content + "(";
    bool done = false;
    auto it = node->children.begin();
    while (!done) {
        result += expr::to_expression_string(*it++);
        done = (it == node->children.end());
        result += done ? ")" : ", ";
    }
    return result;
}

static std::string assignment_to_expression_string(const expr::node_ptr& node) {
    auto lhs = to_expression_string(node->children[0]);
    auto rhs = to_expression_string(node->children[1]);
    return lhs + " = " + rhs;
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

std::string expr::to_expression_string(const expr::node_ptr& root) {
    switch (root->type) {
        case expr::node_t::type_t::BINARY_OP:
            return binary_op_to_expression_string(root);
        case expr::node_t::type_t::UNARY_OP:
            return unary_op_to_expression_string(root);
        case expr::node_t::type_t::NUMBER:
        case expr::node_t::type_t::VARIABLE:
            return root->content;
        case expr::node_t::type_t::FUNCTION_CALL:
            return function_call_to_expression_string(root);
        case expr::node_t::type_t::ASSIGNMENT:
            return assignment_to_expression_string(root);
    }

    // Unreachable
    return "";
}
