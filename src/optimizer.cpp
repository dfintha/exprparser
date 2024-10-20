#include "optimizer.h"
#include "evaluator.h"

#include <cfloat>           // DBL_EPSILON
#include <cmath>            // std::fabs
#include <optional>         // std::optional
#include <sstream>          // std::stringstream, iostream, iomanip

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

static bool are_binary_operands_the_same(
    const std::vector<expr::node_ptr>& operands
) {
    if (operands.size() != 2)
        return false;

    if (operands[0]->type != expr::node_t::type_t::VARIABLE)
        return false;

    if (operands[1]->type != expr::node_t::type_t::VARIABLE)
        return false;

    if (operands[0]->content != operands[1]->content)
        return false;

    return true;
}

static std::string make_number_representation(double value) {
    std::stringstream converter;
    converter << std::noshowpoint << value;
    return converter.str();
}

static std::optional<double> evaluate_child(const expr::node_ptr& child) {
    if (child->type != expr::node_t::type_t::NUMBER)
        return std::nullopt;
    if (auto evaluated = expr::evaluate(child, {}, {}))
        return *evaluated;
    return std::nullopt;
}

static std::optional<expr::optimizer_result> make_optimized_addition(
    expr::node_ptr& original
) {
    for (size_t i = 0; i < original->children.size(); ++i) {
        const auto value = evaluate_child(original->children[i]);

        // Addition of 0 is a no-op both ways.
        if (value && is_near(*value, 0))
            return std::move(original->children[1 - i]);
    }
    return std::nullopt;
}

static std::optional<expr::optimizer_result> make_optimized_subtraction(
    expr::node_ptr& original,
    const expr::location_t location
) {
    // Subtraction of a variable from itself results in 0.
    if (are_binary_operands_the_same(original->children))
        return expr::make_number_literal_node("0", location);

    const auto value_0 = evaluate_child(original->children[0]);
    const auto value_1 = evaluate_child(original->children[1]);

    // Subtraction of 0 is a no-op.
    if (value_1 && is_near(*value_1, 0))
        return std::move(original->children[0]);

    // Subtraction from 0 is a sign change.
    if (value_0 && is_near(*value_0, 0)) {
        return expr::make_unary_operator_node(
            "-",
            std::move(original->children[1]),
            location
        );
    }

    return std::nullopt;
}

static std::optional<expr::optimizer_result> make_optimized_multiplication(
    expr::node_ptr& original,
    const expr::location_t location
) {
    // Multiplication of a variable with itself is its 2nd power.
    if (are_binary_operands_the_same(original->children)) {
        return expr::make_binary_operator_node(
            "^",
            std::move(original->children[0]),
            expr::make_number_literal_node(
                "2",
                original->children[1]->location
            ),
            location
        );
    }

    for (size_t i = 0; i < original->children.size(); ++i) {
        const auto value = evaluate_child(original->children[i]);

        // Multiplication with 0 results in 0.
        if (value && is_near(*value, 0))
            return expr::make_number_literal_node("0", location);

        // Multiplication with 1 is a no-op (both ways).
        if (value && is_near(*value, 1))
            return std::move(original->children[1 - i]);

        // Multiplication with -1 is a sign change (both ways).
        if (value && is_near(*value, -1)) {
            return expr::make_unary_operator_node(
                "-",
                std::move(original->children[1 - i]),
                location
            );
        }
    }

    return std::nullopt;
}

static std::optional<expr::optimizer_result> make_optimized_division(
    expr::node_ptr& original,
    const expr::location_t location
) {
    // Division of a variable with itself results in 1.
    if (are_binary_operands_the_same(original->children)) {
        return  expr::make_number_literal_node("1", location);
    }

    const auto value_0 = evaluate_child(original->children[0]);
    const auto value_1 = evaluate_child(original->children[1]);

    // Division of 0 is always 0.
    if (value_0 && is_near(*value_0, 0))
        return expr::make_number_literal_node("0", location);

    // Division with 1 is a no-op.
    if (value_1 && is_near(*value_1, 1))
        return std::move(original->children[0]);

    // Division with -1 is a sign change.
    if (value_1 && is_near(*value_1, -1)) {
        return expr::make_unary_operator_node(
            "-",
            std::move(original->children[0]),
            location
        );
    }

    return std::nullopt;
}

static std::optional<expr::optimizer_result> make_optimized_exponentiation(
    expr::node_ptr& original,
    const expr::location_t location
) {
    const auto value = evaluate_child(original->children[1]);

    // The 0th power of every number is 1.
    if (value && is_near(*value, 0))
        return expr::make_number_literal_node("1", location);

    // The 1st power of every number is itself.
    if (value && is_near(*value, 1))
        return std::move(original->children[0]);

    return std::nullopt;
}

static expr::optimizer_result make_optimized_binary_op(
    const std::string& operation,
    std::vector<expr::node_ptr> children,
    expr::location_t location
) {
    // We construct a node with the already-optimized children, that we can
    // perform optimizations on.
    auto original = expr::node_ptr{
        new expr::node_t{
            expr::node_t::type_t::BINARY_OP,
            operation,
            std::move(children),
            location
        }
    };

    // If every operand is a number, the expression can be evaluated parse-time.
    if (are_all_children_numbers(original->children)) {
        const auto value = expr::evaluate(original, {}, {});
        if (value) {
            return expr::make_number_literal_node(
                make_number_representation(*value),
                location
            );
        }
    }

    // Otherwise, we'll perform various operation-specific checks to simplify
    // the subexpression.

    if (operation == "+") {
        if (auto optimized = make_optimized_addition(original))
            return std::move(*optimized);
    }

    if (operation == "-") {
        if (auto optimized = make_optimized_subtraction(original, location))
            return std::move(*optimized);
    }

    if (operation == "*") {
        if (auto optimized = make_optimized_multiplication(original, location))
            return std::move(*optimized);
    }

    if (operation == "/") {
        if (auto optimized = make_optimized_division(original, location))
            return std::move(*optimized);
    }

    if (operation == "^") {
        if (auto optimized = make_optimized_exponentiation(original, location))
            return std::move(*optimized);
    }

    // If we could not perform any optimizations, we return with a node with
    // optimized children.
    return original;
}

expr::optimizer_result expr::optimize(const expr::node_ptr& root) {
    // First, we optimize all the children of the node so we can perform later
    // checks on the simplest equivalent subexpression.
    std::vector<expr::node_ptr> children;
    for (auto& child : root->children) {
        if (auto optimized = expr::optimize(child)) {
            children.push_back(std::move(*optimized));
        } else {
            return expr::error {
                expr::error_code::OPTIMIZER_FAILED_TO_OPTIMIZE_CHILD,
                child->location,
                "Failed to optimize child."
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

    // Unary operations on simple numbers are either no-op or a sign change.
    // Both can be evaluated during parse-time.
    if (root->type == expr::node_t::type_t::UNARY_OP) {
        if (root->children[0]->type == expr::node_t::type_t::NUMBER) {
            if (const auto value = expr::evaluate(root, {}, {})) {
                return expr::make_number_literal_node(
                    make_number_representation(*value),
                    root->location
                );
            }
        }
    }

    // Even if no optimization was done, the children of the node were
    // optimized.
    return expr::node_ptr{
        new expr::node_t{
            root->type,
            root->content,
            std::move(children),
            root->location
        }
    };
}
