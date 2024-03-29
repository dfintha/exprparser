#include "optimizer.h"

#include "evaluator.h"
#include <cfloat>
#include <cmath>
#include <iostream>

static bool is_near(double lhs, double rhs) {
    return std::fabs(lhs - rhs) <= DBL_EPSILON;
}

static bool are_all_children_numbers(
    const std::vector<expr::node_ptr>& children
) {
    for (const auto& child : children) {
        if (child->type != expr::node_t::type_t::NUMBER)
            return false;
    }
    return true;
}

static std::optional<double> evaluate_child(const expr::node_ptr& child) {
    if (child->type != expr::node_t::type_t::NUMBER)
        return std::nullopt;
    if (auto evaluated = expr::evaluate(child, {}, {}))
        return *evaluated;
    return std::nullopt;
}

static expr::optimizer_result make_optimized_binary_op(
    const std::string& operation,
    std::vector<expr::node_ptr> children,
    expr::location_t location
) {
    auto original = expr::node_ptr{
        new expr::node_t{
            expr::node_t::type_t::BINARY_OP,
            operation,
            std::move(children),
            location
        }
    };

    // if every operand is a literal, the expression can be evaluated
    if (are_all_children_numbers(original->children)) {
        const auto value = expr::evaluate(original, {}, {});
        if (value)
            return expr::make_number_literal_node(
                std::to_string(*value),
                location
            );
    }

    if (operation == "+" || operation == "-") {
        for (size_t i = 0; i < original->children.size(); ++i) {
            const auto value = evaluate_child(original->children[i]);

            // addition or subtraction with 0 is a no-op
            if (value && is_near(*value, 0))
                return std::move(original->children[1 - i]);
        }
    }

    if (operation == "*" || operation == "/") {
        for (size_t i = 0; i < original->children.size(); ++i) {
            const auto value = evaluate_child(original->children[i]);

            // multiplication or division with 1 is a no-op
            if (value && is_near(*value, 1))
                return std::move(original->children[1 - i]);

            // multiplication or division with -1 is a sign change
            if (value && is_near(*value, -1)) {
                return expr::make_unary_operator_node(
                    "-",
                    std::move(original->children[1 - i]),
                    location
                );
            }
        }
    }

    if (operation == "*") {
        for (size_t i = 0; i < original->children.size(); ++i) {
            const auto value = evaluate_child(original->children[i]);

            // multiplication with 0 results in 0
            if (value && is_near(*value, 0))
                return expr::make_number_literal_node("0", location);
        }
    }

    if (operation == "^") {
        const auto value = evaluate_child(original->children[1]);

        // the 0th power of every number is 1
        if (value && is_near(*value, 0))
            return expr::make_number_literal_node("1", location);

        // the 1st power of every number is itself
        if (value && is_near(*value, 1))
            return std::move(original->children[0]);
    }

    return original;
}

expr::optimizer_result expr::optimize(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children;
    for (auto& child : root->children) {
        if (auto optimized = expr::optimize(child)) {
            children.push_back(std::move(*optimized));
        } else {
            return expr::error {
                expr::error_code::OPTIMIZER_FAILED_TO_OPTIMIZE_CHILD,
                child->location,
                "failed to optimize child"
            };
        }
    }

    if (root->type == expr::node_t::type_t::BINARY_OP) {
        return make_optimized_binary_op(
            root->content,
            std::move(children),
            root->location
        );
    }

    if (root->type == expr::node_t::type_t::UNARY_OP) {
        if (root->children[0]->type == expr::node_t::type_t::NUMBER) {
            if (const auto value = expr::evaluate(root, {}, {})) {
                return expr::make_number_literal_node(
                    std::to_string(*value), root->location
                );
            }
        }
    }

    return expr::node_ptr{
        new expr::node_t{
            root->type,
            root->content,
            std::move(children),
            root->location
        }
    };
}
