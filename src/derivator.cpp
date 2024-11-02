#include "derivator.h"
#include "optimizer.h"

#include <unordered_map>    // std::unordered_map

using namespace std::literals;

using derivator_t = expr::derivator_result (*)(const expr::node_ptr&);
using derivator_table = std::unordered_map<std::string, derivator_t>;

static expr::node_ptr clone_node(const expr::node_ptr& node) {
    std::vector<expr::node_ptr> children;
    for (const auto& child : node->children) {
        children.push_back(clone_node(child));
    }

    return expr::node_ptr(
        new expr::node_t{node->type, node->content, std::move(children), {}}
    );
}

static void erase_location(expr::node_ptr& node) {
    node->location = {};
    for (auto& child : node->children) {
        erase_location(child);
    }
}

static expr::derivator_result derive_primary(const expr::node_ptr& root) {
    if (root->type == expr::node_t::type_t::NUMBER)
        return expr::make_number_literal_node("0", {});

    if (root->type == expr::node_t::type_t::BOOLEAN)
        return expr::make_number_literal_node("0", {});

    if (root->type == expr::node_t::type_t::VARIABLE)
        return expr::make_number_literal_node("1", {});

    return expr::error{
        expr::error_code::DERIVATOR_GENERAL_ERROR,
        root->location,
        "Attempted derivation of non-primary node as primary."
    };
}

static expr::derivator_result derive_unary_op(const expr::node_ptr& root) {
    auto operand = derive(clone_node(root->children[0]));
    if (!operand)
        return operand;

    // The derivative of unary operators is the same unary operation performed
    // on the operand's derivative.
    return expr::make_unary_operator_node(
        root->content,
        std::move(*operand),
        {}
    );
}

static expr::derivator_result derive_binary_op(const expr::node_ptr& root) {
    auto left_d = derive(clone_node(root->children[0]));
    if (!left_d)
        return left_d;

    auto right_d = derive(clone_node(root->children[1]));
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
                {}
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
                    {}
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
                    {}
                );
            }

            auto first = expr::make_binary_operator_node(
                "*",
                std::move(*left_d),
                std::move(*right),
                {}
            );

            auto second = expr::make_binary_operator_node(
                "*",
                std::move(*left),
                std::move(*right_d),
                {}
            );

            // Otherwise, we have to apply the generic derivation rule for
            // multiplication: "(f * g)' = f' * g + f * g'".
            if (root->content == "*") {
                return expr::make_binary_operator_node(
                    "+",
                    std::move(first),
                    std::move(second),
                    {}
                );
            }

            // If we have a division at hand, we use the generic derivation rule
            // for that: "(f / g)' = (f' * g - f * g') / g^2"

            auto third = expr::make_binary_operator_node(
                "-",
                std::move(first),
                std::move(second),
                {}
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
                    expr::make_number_literal_node("2", {}),
                    {}
                ),
                {}
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
                            expr::make_number_literal_node("1", {}),
                            {}
                        ),
                        {}
                    ),
                    {}
                );
            }

            // Otherwise we apply the generic derivation rule for powers:
            // "x^y * ln(x)".

            std::vector<expr::node_ptr> children;
            children.emplace_back(clone_node(root->children[0]));

            return expr::make_binary_operator_node(
                "*",
                clone_node(root),
                expr::make_function_call_node("ln", std::move(children), {}),
                {}
            );
        }
    }

    return expr::error{
        expr::error_code::DERIVATOR_GENERAL_ERROR,
        root->location,
        "Unsupported binary operator: '"s + root->content +  "'"
    };
}

static expr::derivator_result derive_sin(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children;
    children.emplace_back(clone_node(root->children[0]));

    return expr::make_function_call_node(
        "cos",
        std::move(children),
        {}
    );
}

static expr::derivator_result derive_cos(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children;
    children.emplace_back(clone_node(root->children[0]));

    return expr::make_unary_operator_node(
        "-",
        expr::make_function_call_node("sin", std::move(children), {}),
        {}
    );
}

static expr::derivator_result derive_tan(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children;
    children.push_back(clone_node(root->children[0]));

    return expr::make_binary_operator_node(
        "/",
        expr::make_number_literal_node("1", {}),
        expr::make_binary_operator_node(
            "^",
            expr::make_function_call_node(
                "cos",
                std::move(children),
                {}
            ),
            expr::make_number_literal_node("2", {}),
            {}
        ),
        {}
    );
}

static expr::derivator_result derive_ctg(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children;
    children.push_back(clone_node(root->children[0]));

    return expr::make_unary_operator_node(
        "-",
        expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", {}),
            expr::make_binary_operator_node(
                "^",
                expr::make_function_call_node(
                    "sin",
                    std::move(children),
                    {}
                ),
                expr::make_number_literal_node("2", {}),
                {}
            ),
            {}
        ),
        {}
    );
}

static expr::derivator_result derive_sec(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children_sec;
    children_sec.push_back(clone_node(root->children[0]));

    std::vector<expr::node_ptr> children_tan;
    children_tan.push_back(clone_node(root->children[0]));

    return expr::make_binary_operator_node(
        "*",
        expr::make_function_call_node("sec", std::move(children_sec), {}),
        expr::make_function_call_node("tan", std::move(children_tan), {}),
        {}
    );
}

static expr::derivator_result derive_csc(const expr::node_ptr& root) {
    std::vector<expr::node_ptr> children_csc;
    children_csc.push_back(clone_node(root->children[0]));

    std::vector<expr::node_ptr> children_ctg;
    children_ctg.push_back(clone_node(root->children[0]));

    return expr::make_binary_operator_node(
        "*",
        expr::make_unary_operator_node(
            "-",
            expr::make_function_call_node("csc", std::move(children_csc), {}),
            {}
        ),
        expr::make_function_call_node("ctg", std::move(children_ctg), {}),
        {}
    );
}

static expr::derivator_result derive_nonderivables(const expr::node_ptr& root) {
    return expr::error{
        expr::error_code::DERIVATOR_FUNCTION_NOT_DERIVABLE,
        root->location,
        "Function "s + root->content + "(...) can not be derived."
    };
}

static expr::derivator_result derive_logarithms(const expr::node_ptr& root) {
    // The derivative of ln(x) can be simplified to 1/x.
    if (root->content == "ln") {
        return expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", {}),
            clone_node(root->children[0]),
            {}
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
                expr::error_code::DERIVATOR_WRONG_ARGUMENT_COUNT,
                root->location,
                "Function log(x, base) takes 2 argument(s)."
            };

        auto base = expr::optimize(root->children[1]);
        if (!base)
            return base;

        erase_location(*base);
        if ((*base)->type != expr::node_t::type_t::NUMBER) {
            return expr::error{
                expr::error_code::DERIVATOR_CANT_BE_DONE_AT_PARSE_TIME,
                root->location,
                "Can't derive logarithm functions with an unknown base at "
                "parse time."
            };
        }

        base_literal = (*base)->content;
    }

    std::vector<expr::node_ptr> children;
    children.push_back(expr::make_number_literal_node(base_literal, {}));

    return expr::make_binary_operator_node(
        "*",
        expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", {}),
            expr::make_function_call_node("ln", std::move(children), {}),
            {}
        ),
        expr::make_binary_operator_node(
            "/",
            expr::make_number_literal_node("1", {}),
            clone_node(root->children[0]),
            {}
        ),
        {}
    );
}

const derivator_table& function_derivators() {
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

static expr::derivator_result derive_function_call(const expr::node_ptr& root) {
    const auto& derivators = function_derivators();
    if (derivators.count(root->content) != 0)
        return derivators.at(root->content)(root);

    return expr::error{
        expr::error_code::DERIVATOR_GENERAL_ERROR,
        root->location,
        "Derivation is not implemented for "s + root->content + "(...)."
    };
}

static expr::derivator_result derive_assignment(const expr::node_ptr& root) {
    return derive(root->children[1]);
}

expr::derivator_result expr::derive(const expr::node_ptr& root) {
#define FOR_NODE(NODE_TYPE, ...)                        \
    case expr::node_t::type_t::NODE_TYPE: {             \
        auto result = __VA_ARGS__;                      \
        if (!result.has_value())                        \
            return result;                              \
        return expr::optimize(std::move(*result));      \
    } break                                             \

    switch (root->type) {
        FOR_NODE(BINARY_OP, derive_binary_op(root));
        FOR_NODE(UNARY_OP, derive_unary_op(root));
        FOR_NODE(NUMBER, derive_primary(root));
        FOR_NODE(BOOLEAN, derive_primary(root));
        FOR_NODE(VARIABLE, derive_primary(root));
        FOR_NODE(FUNCTION_CALL, derive_function_call(root));
        FOR_NODE(ASSIGNMENT, derive_assignment(root));
    }

#undef FOR_NODE

    return expr::error{
        expr::error_code::DERIVATOR_GENERAL_ERROR,
        root->location,
        "Function derivation is not exhausitve for all node types."
    };
}
