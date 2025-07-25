#include "optimizer.h"
#include "evaluator.h"
#include "utility.h"

#include <optional>         // std::optional
#include <sstream>          // std::stringstream, iostream, iomanip

static bool are_all_children_numbers(
    const std::vector<expr::node_ptr>& children
) noexcept {
    for (const auto& child : children) {
        if (child->type != expr::node_t::type_t::NUMBER)
            return false;
    }
    return true;
}

static bool are_binary_operands_the_same(
    const std::vector<expr::node_ptr>& operands
) noexcept {
    return operands.size() == 2 && *operands[0] == *operands[1];
}

static std::string make_number_representation(expr::quantity value) {
    std::stringstream converter;
    converter << std::noshowpoint << value.value;
    return converter.str();
}

static std::optional<expr::optimizer_result> make_optimized_addition(
    expr::node_ptr& original
) {
    const expr::evaluator_result values[2] = {
        expr::evaluate_parse_time(original->children[0]),
        expr::evaluate_parse_time(original->children[1]),
    };

    if (!values[0] || !values[1])
        return std::nullopt;

    for (size_t i = 0; i < original->children.size(); ++i) {
        // Addition of 0 is a no-op both ways, regardless of units.
        if (expr::is_near(values[i]->value, 0))
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

    const expr::evaluator_result values[2] = {
        expr::evaluate_parse_time(original->children[0]),
        expr::evaluate_parse_time(original->children[1]),
    };

    // Subtraction of 0 is a no-op, regardless of units.
    if (values[1] && expr::is_near(values[1]->value, 0))
        return std::move(original->children[0]);

    // Subtraction from 0 is a sign change, regardless of units.
    if (values[0] && expr::is_near(values[0]->value, 0)) {
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
        const auto value = expr::evaluate_parse_time(original->children[i]);

        // Multiplication with 0 results in 0, regardless of units.
        if (value && expr::is_near(value->value, 0))
            return expr::make_number_literal_node("0", location);

        // Multiplication with scalar 1 is a no-op (both ways).
        if (value && value->is_scalar() && expr::is_near(value->value, 1))
            return std::move(original->children[1 - i]);

        // Multiplication with scalar -1 is a sign change (both ways).
        if (value && value->is_scalar() && expr::is_near(value->value, -1)) {
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

    const expr::evaluator_result values[2] = {
        expr::evaluate_parse_time(original->children[0]),
        expr::evaluate_parse_time(original->children[1]),
    };

    // Division of 0 is always 0.
    if (values[0] && expr::is_near(values[0]->value, 0))
        return expr::make_number_literal_node("0", location);

    // Division with scalar 1 is a no-op.
    if (values[1] && values[1]->is_scalar() && expr::is_near(values[1]->value, 1))
        return std::move(original->children[0]);

    // Division with scalar -1 is a sign change.
    if (values[1] && values[1]->is_scalar() && expr::is_near(values[1]->value, -1)) {
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
    const auto value = expr::evaluate_parse_time(original->children[1]);

    // If the exponent is not a scalar, the expression is malformed.
    if (!value->is_scalar())
        return std::nullopt;

    // The 0th power of every number is 1.
    if (value && expr::is_near(value->value, 0))
        return expr::make_number_literal_node("1", location);

    // The 1st power of every number is itself.
    if (value && expr::is_near(value->value, 1))
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
        const auto value = expr::evaluate_parse_time(original);
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

static std::vector<expr::node_ptr> optimize_children(
    const expr::node_ptr& node
) {
    std::vector<expr::node_ptr> result;
    for (auto& child : node->children) {
        if (auto optimized = expr::optimize(child)) {
            result.push_back(std::move(*optimized));
        } else {
            return result;
        }
    }
    return result;
}

expr::optimizer_result expr::optimize(const expr::node_ptr& root) {
    // First, we optimize all the children of the node so we can perform later
    // checks on the simplest equivalent subexpression.
    auto children = optimize_children(root);
    if (children.size() != root->children.size()) {
        return expr::error {
            .code = expr::error_code::OPTIMIZER_FAILED_TO_OPTIMIZE_CHILD,
            .location = root->children[children.size()]->location,
            .description = "Failed to optimize child."
        };
    }

    // We don't want to optimize assignments away.
    if (root->type == expr::node_t::type_t::ASSIGNMENT) {
        return expr::make_assignment_node(
            std::move(children[0]),
            std::move(children[1]),
            root->location
        );
    }

    // We shortcut the whole optimization if the expression can be evaluated
    // parse-time. We do this after children are optimized so some variables
    // may be optimized out (e.g. x - x is always 0 but if we try to evaluate
    // the tree as-is we's fail since x can not be evaluated parse-time).
    auto preoptimized = std::unique_ptr<expr::node_t> {
        new expr::node_t {
            .type = root->type,
            .content = root->content,
            .children = optimize_children(root),
            .location = root->location
        }
    };
    if (auto evaluated = expr::evaluate_parse_time(preoptimized)) {
        return expr::make_number_literal_node(
            make_number_representation(*evaluated),
            root->location
        );
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
            if (const auto value = expr::evaluate_parse_time(root)) {
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
            .type = root->type,
            .content = root->content,
            .children = std::move(children),
            .location = root->location
        }
    };
}
