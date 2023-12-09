#include "evaluator.h"

#include <cmath>
#include <functional>

namespace expr {
    std::optional<double> evaluate(
        const node_ptr& node,
        const symbol_table& symbols
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
                const auto left = evaluate(node->children[0], symbols);
                const auto right = evaluate(node->children[1], symbols);
                if (!left || !right) {
                    return std::nullopt;
                }
                const auto& f = binary.at(node->content);
                return f(*left, *right);
            }
            case node_t::type_t::UNARY_OP:
                if (const auto operand = evaluate(node->children[0], symbols)) {
                    const auto& f = unary.at(node->content);
                    return f(*operand);
                }
                return std::nullopt;
            case node_t::type_t::NUMBER: {
                const auto result = strtod(node->content.c_str(), nullptr);
                return result;
            }
            case node_t::type_t::BOOLEAN:
                return node->content == "true" ? 1 : 0;
            case node_t::type_t::IDENTIFIER:
                if (symbols.find(node->content) == symbols.end())
                    return std::nullopt;
                return symbols.at(node->content);
        }
    }
}
