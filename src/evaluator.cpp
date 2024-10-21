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
            "Failed to evaluate function arguments for '"s + name + "()'."
        };
    }
    return function(evaluated, location);
}

static int get_precedence_score(const expr::node_ptr& node) {
    switch (node->type) {
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
        case expr::node_t::type_t::BOOLEAN:
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
    const bool child_num = node->children[0]->type == expr::node_t::type_t::NUMBER;
    const bool child_var = node->children[0]->type == expr::node_t::type_t::VARIABLE;
    const bool child_bool = node->children[0]->type == expr::node_t::type_t::BOOLEAN;
    const bool child_primary = child_num || child_var || child_bool;
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
            {"%", [](double lhs, double rhs) { return std::fmod(lhs, rhs); }},
            {"^", [](double lhs, double rhs) { return std::pow(lhs, rhs); }},
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
                        "Failed to evaluate operand."
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
                    "Failed to evaluate operand."
                };
            }
            case node_t::type_t::NUMBER: {
                if (auto bin = parse_binary_number(node->content))
                    return *bin;
                if (auto oct = parse_octal_number(node->content))
                    return *oct;
                return std::strtod(node->content.c_str(), nullptr);
            }
            case node_t::type_t::BOOLEAN:
                return node->content == "true" ? 1.0 : 0.0;
            case node_t::type_t::VARIABLE:
                if (symbols.find(node->content) == symbols.end()) {
                    return error{
                        error_code::EVALUATOR_UNDEFINED_VARIABLE,
                        node->location,
                        "Undefined variable '"s + node->content + "'."
                    };
                }
                return symbols.at(node->content);
            case node_t::type_t::FUNCTION_CALL:
                if (functions.find(node->content) == functions.end()) {
                    const auto& where = node->location.begin;
                    return error{
                        error_code::EVALUATOR_UNDEFINED_FUNCTION,
                        location_t{where, where + node->content.length() - 1},
                        "Undefined function '"s + node->content + "'."
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
        }

        // Unreachable
        return error{
            error_code::EVALUATOR_REACHED_UNREACHABLE_CODE_PATH,
            location_t{0, 0},
            "The evaluator has reached a supposedly unreachable code path."
        };
    }

    std::string to_expression_string(const node_ptr& root) {
        switch (root->type) {
            case expr::node_t::type_t::BINARY_OP:
                return binary_op_to_expression_string(root);
            case expr::node_t::type_t::UNARY_OP:
                return unary_op_to_expression_string(root);
            case expr::node_t::type_t::NUMBER:
            case expr::node_t::type_t::BOOLEAN:
            case expr::node_t::type_t::VARIABLE:
                return root->content;
            case expr::node_t::type_t::FUNCTION_CALL:
                return function_call_to_expression_string(root);
        }

        // Unreachable
        return "";
    }
}
