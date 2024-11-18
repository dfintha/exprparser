#include "derivator.h"
#include "optimizer.h"

#include <unordered_map>    // std::unordered_map

using derivator_t = expr::derivator_result (*)(
    const expr::node_ptr&,
    std::string_view
);

using derivator_table = std::unordered_map<std::string, derivator_t>;

static constexpr expr::location_t empty_location = expr::location_t{
    .begin = 0,
    .end = 0
};

static expr::node_ptr clone_node(const expr::node_ptr& node) {
    std::vector<expr::node_ptr> children;
    for (const auto& child : node->children) {
        children.push_back(clone_node(child));
    }

    return expr::node_ptr(
        new expr::node_t{
            .type = node->type,
            .content = node->content,
            .children = std::move(children),
            .location = empty_location
        }
    );
}

static void erase_location(expr::node_ptr& node) {
    node->location = empty_location;
    for (auto& child : node->children) {
        erase_location(child);
    }
}

static expr::derivator_result derive_primary(
    const expr::node_ptr& root,
    std::string_view variable
) {
    if (root->type == expr::node_t::type_t::NUMBER)
        return expr::make_number_literal_node("0", empty_location);

    if (root->type == expr::node_t::type_t::VARIABLE) {
        if (root->content == variable)
            return expr::make_number_literal_node("1", empty_location);
        return clone_node(root);
    }

    return expr::error{
        .code = expr::error_code::DERIVATOR_GENERAL_ERROR,
        .location = root->location,
        .description = "Attempted derivation of non-primary node as primary."
    };
}

static expr::derivator_result derive_unary_op(
    const expr::node_ptr& root,
    std::string_view variable
) {
    auto operand = derive(clone_node(root->children[0]), variable);
    if (!operand)
        return operand;

    // The derivative of unary operators is the same unary operation performed
    // on the operand's derivative.
    return expr::make_unary_operator_node(
        root->content,
        std::move(*operand),
        empty_location
    );
}

static expr::derivator_result derive_binary_op(
    const expr::node_ptr& root,
    std::string_view variable
) {
    auto left_d = derive(clone_node(root->children[0]), variable);
    if (!left_d)
        return left_d;

    auto right_d = derive(clone_node(root->children[1]), variable);
    if (!right_d)
        return right_d;

    switch (root->content[0]) {
        case '+':
        case '-': {
            // The derivative of addition and subtraction is the addition or
            // subtraction of the operands' derivatives.
            return expr::make_binary_operator_node(
                root->content,
                std::move(*left_d),
                std::move(*right_d),
                empty_location
            );
        }
        break;

        case '*':
        case '/': {
            // If the left operand is a number, we only have to derive the right
            // operand's subexpression.
            auto left = expr::optimize(root->children[0]);
            if (!left)
                return left;

            erase_location(*left);
            if ((*left)->type == expr::node_t::type_t::NUMBER) {
                return expr::make_binary_operator_node(
                    root->content,
                    std::move(*left),
                    std::move(*right_d),
                    empty_location
                );
            }

            // If the right operand is a number, we only have to derive the
            // left operand's subexpression.
            auto right = expr::optimize(root->children[1]);
            if (!right)
                return right;

            erase_location(*right);
            if ((*right)->type == expr::node_t::type_t::NUMBER) {
                return expr::make_binary_operator_node(
                    root->content,
                    std::move(*right),
                    std::move(*left_d),
                    empty_location
                );
            }

            auto first = expr::make_binary_operator_node(
                "*",
                std::move(*left_d),
                std::move(*right),
                empty_location
            );

            auto second = expr::make_binary_operator_node(
                "*",
                std::move(*left),
                std::move(*right_d),
                empty_location
            );

            // Otherwise, we have to apply the generic derivation rule for
            // multiplication: "(f * g)' = f' * g + f * g'".
            if (root->content == "*") {
                return expr::make_binary_operator_node(
                    "+",
                    std::move(first),
                    std::move(second),
                    empty_location
                );
            }

            // If we have a division at hand, we use the generic derivation rule
            // for that: "(f / g)' = (f' * g - f * g') / g^2"

            auto third = expr::make_binary_operator_node(
                "-",
                std::move(first),
                std::move(second),
                empty_location
            );

            auto fourth = expr::optimize(root->children[1]);
            if (!fourth)
                return fourth;

            erase_location(*fourth);
            return expr::make_binary_operator_node(
                "/",
                std::move(third),
                expr::make_binary_operator_node(
                    "^",
                    std::move(*fourth),
                    expr::make_number_literal_node("2", empty_location),
                    empty_location
                ),
                empty_location
            );
        }

        case '^': {
            // If the right operand is a number, we can apply the derivation
            // rule for simple powers: "n * x^(n - 1)".
            if (root->children[1]->type == expr::node_t::type_t::NUMBER) {
                return expr::make_binary_operator_node(
                    "*",
                    clone_node(root->children[1]),
                    expr::make_binary_operator_node(
                        "^",
                        clone_node(root->children[0]),
                        expr::make_binary_operator_node(
                            "-",
                            clone_node(root->children[1]),
                            expr::make_number_literal_node("1", empty_location),
                            empty_location
                        ),
                        empty_location
                    ),
                    empty_location
                );
            }

            // Otherwise we apply the generic derivation rule for powers:
            // "x^y * ln(x)".

            std::vector<expr::node_ptr> children;
            children.emplace_back(clone_node(root->children[0]));

            return expr::make_binary_operator_node(
                "*",
                clone_node(root),
                expr::make_function_call_node(
                    "ln",
                    std::move(children),
                    empty_location
                ),
                empty_location
            );
        }
    }

    return expr::error{
        .code = expr::error_code::DERIVATOR_GENERAL_ERROR,
        .location = root->location,
        .description = "Unsupported binary operator: '" + root->content +  "'"
    };
}

static expr::derivator_result derive_sin(
    const expr::node_ptr& root,
    std::string_view
) {
    std::vector<expr::node_ptr> children;
    children.emplace_back(clone_node(root->children[0]));

    return expr::make_function_call_node(
        "cos",
        std::move(children),
        empty_location
    );
}

static expr::derivator_result derive_cos(
    const expr::node_ptr& root,
    std::string_view
) {
    std::vector<expr::node_ptr> children;
    children.emplace_back(clone_node(root->children[0]));

    return expr::make_unary_operator_node(
        "-",
        expr::make_function_call_node(
            "sin",
            std::move(children),
            empty_location
        ),
        empty_location
    );
}

static expr::derivator_result derive_tan(
    const expr::node_ptr& root,
    std::string_view
) {
    std::vector<expr::node_ptr> children;
    children.push_back(clone_node(root->children[0]));

    return expr::make_binary_operator_node(
        "/",
        expr::make_number_literal_node("1", empty_location),
        expr::make_binary_operator_node(
            "^",
            expr::make_function_call_node(
                "cos",
                std::move(children),
                empty_location
            ),
            expr::make_number_literal_node("2", empty_location),
            empty_location
        ),
        empty_location
    );
}

static expr::derivator_result derive_ctg(
    const expr::node_ptr& root,
    std::string_view
) {
    std::vector<expr::node_ptr> children;
    children.push_back(clone_node(root->children[0]));

    return expr::make_unary_operator_node(
        "-",
        expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", empty_location),
            expr::make_binary_operator_node(
                "^",
                expr::make_function_call_node(
                    "sin",
                    std::move(children),
                    empty_location
                ),
                expr::make_number_literal_node("2", empty_location),
                empty_location
            ),
            empty_location
        ),
        empty_location
    );
}

static expr::derivator_result derive_sec(
    const expr::node_ptr& root,
    std::string_view
) {
    std::vector<expr::node_ptr> children_sec;
    children_sec.push_back(clone_node(root->children[0]));

    std::vector<expr::node_ptr> children_tan;
    children_tan.push_back(clone_node(root->children[0]));

    return expr::make_binary_operator_node(
        "*",
        expr::make_function_call_node(
            "sec",
            std::move(children_sec),
            empty_location
        ),
        expr::make_function_call_node(
            "tan",
            std::move(children_tan),
            empty_location
        ),
        empty_location
    );
}

static expr::derivator_result derive_csc(
    const expr::node_ptr& root,
    std::string_view
) {
    std::vector<expr::node_ptr> children_csc;
    children_csc.push_back(clone_node(root->children[0]));

    std::vector<expr::node_ptr> children_ctg;
    children_ctg.push_back(clone_node(root->children[0]));

    return expr::make_binary_operator_node(
        "*",
        expr::make_unary_operator_node(
            "-",
            expr::make_function_call_node(
                "csc",
                std::move(children_csc),
                empty_location
            ),
            empty_location
        ),
        expr::make_function_call_node(
            "ctg",
            std::move(children_ctg),
            empty_location
        ),
        empty_location
    );
}

static expr::derivator_result derive_nonderivables(
    const expr::node_ptr& root,
    std::string_view
) {
    return expr::error{
        .code = expr::error_code::DERIVATOR_FUNCTION_NOT_DERIVABLE,
        .location = root->location,
        .description = "Function " + root->content + "(...) can't be derived."
    };
}

static expr::derivator_result derive_logarithms(
    const expr::node_ptr& root,
    std::string_view
) {
    // The derivative of ln(x) can be simplified to 1/x.
    if (root->content == "ln") {
        return expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", empty_location),
            clone_node(root->children[0]),
            empty_location
        );
    }

    // Otherwise we apply the generic derivation rule for logarithms if we can
    // collapse the base to a literal during parse time: "1/ln(y) * 1/x".

    std::string base_literal;
    if (root->content == "log2" || root->content == "log10") {
        base_literal = root->content.substr(3);
    } else {
        if (root->children.size() != 2)
            return expr::error{
                .code = expr::error_code::DERIVATOR_WRONG_ARGUMENT_COUNT,
                .location = root->location,
                .description = "Function log(x, base) takes 2 argument(s)."
            };

        auto base = expr::optimize(root->children[1]);
        if (!base)
            return base;

        erase_location(*base);
        if ((*base)->type != expr::node_t::type_t::NUMBER) {
            return expr::error{
                .code = expr::error_code::DERIVATOR_CANT_BE_DONE_AT_PARSE_TIME,
                .location = root->location,
                .description = "Can't derive logarithm functions with an "
                               "unknown base at parse time."
            };
        }

        base_literal = (*base)->content;
    }

    std::vector<expr::node_ptr> children;
    children.push_back(
        expr::make_number_literal_node(base_literal, empty_location)
    );

    return expr::make_binary_operator_node(
        "*",
        expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", empty_location),
            expr::make_function_call_node(
                "ln",
                std::move(children),
                empty_location
            ),
            empty_location
        ),
        expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", empty_location),
            clone_node(root->children[0]),
            empty_location
        ),
        empty_location
    );
}

static const derivator_table& function_derivators() {
    static const auto table = derivator_table{
        {"sin", derive_sin},
        {"cos", derive_cos},
        {"tan", derive_tan},
        {"ctg", derive_ctg},
        {"sec", derive_sec},
        {"csc", derive_csc},
        {"round", derive_nonderivables},
        {"floor", derive_nonderivables},
        {"ceil", derive_nonderivables},
        {"abs", derive_nonderivables},
        {"ln", derive_logarithms},
        {"log2", derive_logarithms},
        {"log10", derive_logarithms},
        {"log", derive_logarithms},
        {"sgn", derive_nonderivables}
    };
    return table;
}

static expr::derivator_result derive_function_call(
    const expr::node_ptr& root,
    std::string_view variable
) {
    const auto& derivators = function_derivators();
    if (derivators.count(root->content) != 0)
        return derivators.at(root->content)(root, variable);

    return expr::error{
        .code = expr::error_code::DERIVATOR_GENERAL_ERROR,
        .location = root->location,
        .description = "Derivation is not implemented for " + root->content +
                       "(...)."
    };
}

static expr::derivator_result derive_assignment(
    const expr::node_ptr& root,
    std::string_view variable
) {
    return derive(root->children[1], variable);
}

expr::derivator_result expr::derive(
    const expr::node_ptr& root,
    std::string_view variable
) {
#define FOR_NODE(NODE_TYPE, ...)                        \
    case expr::node_t::type_t::NODE_TYPE: {             \
        auto result = __VA_ARGS__;                      \
        if (!result.has_value())                        \
            return result;                              \
        return expr::optimize(std::move(*result));      \
    } break                                             \

    switch (root->type) {
        FOR_NODE(BINARY_OP, derive_binary_op(root, variable));
        FOR_NODE(UNARY_OP, derive_unary_op(root, variable));
        FOR_NODE(NUMBER, derive_primary(root, variable));
        FOR_NODE(VARIABLE, derive_primary(root, variable));
        FOR_NODE(FUNCTION_CALL, derive_function_call(root, variable));
        FOR_NODE(ASSIGNMENT, derive_assignment(root, variable));
    }

#undef FOR_NODE

    return expr::error{
        .code = expr::error_code::DERIVATOR_GENERAL_ERROR,
        .location = root->location,
        .description = "Derivation is not exhausitve for all node types."
    };
}
