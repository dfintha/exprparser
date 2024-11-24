#include "node.h"

#include <iostream>         // std::ostream

enum precedence_t {
    INVALID_PRECEDENCE = 0,
    ASSIGNMENT_PRECEDENCE = 1,
    TERM_PRECEDENCE = 2,
    FACTOR_PRECEDENCE = 3,
    POWER_PRECEDENCE = 4,
    UNARY_PRECEDENCE = 5,
    PRIMARY_PRECEDENCE = 6
};

static precedence_t get_precedence_score(const expr::node_ptr& node) noexcept {
    switch (node->type) {
        case expr::node_t::type_t::ASSIGNMENT:
            return ASSIGNMENT_PRECEDENCE;
        case expr::node_t::type_t::BINARY_OP: {
            if (node->content == "+" || node->content == "-")
                return TERM_PRECEDENCE;
            if (node->content == "*" || node->content == "/")
                return FACTOR_PRECEDENCE;
            return POWER_PRECEDENCE;
        }
        case expr::node_t::type_t::UNARY_OP:
            return UNARY_PRECEDENCE;
        case expr::node_t::type_t::FUNCTION_CALL:
        case expr::node_t::type_t::NUMBER:
        case expr::node_t::type_t::VARIABLE:
            return PRIMARY_PRECEDENCE;
    }

    // Unreachable
    return INVALID_PRECEDENCE;
}

static std::string binary_op_to_expression_string(const expr::node_ptr& node) {
    const auto left_precedence = get_precedence_score(node->children[0]);
    const auto right_precedence = get_precedence_score(node->children[1]);
    const auto parent_precedence = get_precedence_score(node);
    const bool left_parentheses = left_precedence < parent_precedence;
    const bool right_parentheses = right_precedence < parent_precedence;
    return (left_parentheses ? "(" : "")
        + expr::to_expression_string(node->children[0])
        + (left_parentheses ? ")" : "")
        + " " + node->content + " "
        + (right_parentheses ? "(" : "")
        + expr::to_expression_string(node->children[1])
        + (right_parentheses ? ")" : "");
}

static std::string unary_op_to_expression_string(const expr::node_ptr& node) {
    const auto child_precedence = get_precedence_score(node->children[0]);
    const bool parentheses = child_precedence < UNARY_PRECEDENCE;
    return node->content
        + (parentheses ? "(" : "")
        + expr::to_expression_string(node->children[0])
        + (parentheses ? ")" : "");
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

bool expr::operator==(
    const expr::node_t& lhs,
    const expr::node_t& rhs
) noexcept {
    if (lhs.type != rhs.type)
        return false;

    const size_t size = lhs.children.size();
    if (rhs.children.size() != size)
        return false;

    if (lhs.content != rhs.content)
        return false;

    for (size_t i = 0; i < size; ++i) {
        if (*lhs.children[i] != *rhs.children[i])
            return false;
    }

    return true;
}

bool expr::operator!=(
    const expr::node_t& lhs,
    const expr::node_t& rhs
) noexcept {
    return !(lhs == rhs);
}

expr::node_ptr expr::make_number_literal_node(
    std::string content,
    const expr::location_t& location
) {
    return std::unique_ptr<expr::node_t>(
        new expr::node_t{
            .type = expr::node_t::type_t::NUMBER,
            .content = std::move(content),
            .children = {},
            .location = location
        }
    );
}

expr::node_ptr expr::make_variable_node(
    std::string content,
    const expr::location_t& location
) {
    return std::unique_ptr<expr::node_t>(
        new expr::node_t{
            .type = expr::node_t::type_t::VARIABLE,
            .content = std::move(content),
            .children = {},
            .location = location
        }
    );
}

expr::node_ptr expr::make_unary_operator_node(
    std::string content,
    expr::node_ptr&& operand,
    const expr::location_t& location
) {
    expr::node_ptr result = std::unique_ptr<expr::node_t>(
        new expr::node_t{
            .type = expr::node_t::type_t::UNARY_OP,
            .content = std::move(content),
            .children = {},
            .location = location
        }
    );

    result->children.push_back(std::move(operand));

    return result;
}

expr::node_ptr expr::make_binary_operator_node(
    std::string content,
    expr::node_ptr&& left,
    expr::node_ptr&& right,
    const expr::location_t& location
) {
    expr::node_ptr result = std::unique_ptr<expr::node_t>(
        new expr::node_t{
            .type = expr::node_t::type_t::BINARY_OP,
            .content = std::move(content),
            .children = {},
            .location = location
        }
    );

    result->children.push_back(std::move(left));
    result->children.push_back(std::move(right));

    return result;
}

expr::node_ptr expr::make_function_call_node(
    std::string content,
    std::vector<expr::node_ptr>&& parameters,
    const expr::location_t& location
) {
    return std::unique_ptr<expr::node_t>(
        new expr::node_t{
            .type = expr::node_t::type_t::FUNCTION_CALL,
            .content = std::move(content),
            .children = std::move(parameters),
            .location = location
        }
    );
}

expr::node_ptr expr::make_assignment_node(
    expr::node_ptr&& left,
    expr::node_ptr&& right,
    const expr::location_t& location
) {
    expr::node_ptr result = std::unique_ptr<expr::node_t>(
        new expr::node_t{
            .type = expr::node_t::type_t::ASSIGNMENT,
            .content = "=",
            .children = {},
            .location = location
        }
    );

    result->children.push_back(std::move(left));
    result->children.push_back(std::move(right));

    return result;
}

std::ostream& operator<<(std::ostream& stream, expr::node_t::type_t type) {
    switch (type) {
        case expr::node_t::type_t::BINARY_OP:
            return stream << "BinaryOperator";
        case expr::node_t::type_t::UNARY_OP:
            return stream << "UnaryOperator";
        case expr::node_t::type_t::NUMBER:
            return stream << "NumberLiteral";
        case expr::node_t::type_t::VARIABLE:
            return stream << "Variable";
        case expr::node_t::type_t::FUNCTION_CALL:
            return stream << "FunctionCall";
        case expr::node_t::type_t::ASSIGNMENT:
            return stream << "Assignment";
    }

    // Unreachable
    return stream;
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

std::ostream& operator<<(std::ostream& stream, const expr::node_ptr& node) {
    static std::string indent = "";
    stream << indent << node->type
           << "('" << node->content
           << "'@" << node->location
           << ')' << std::endl;
    indent += "  ";
    for (const auto& child : node->children) {
        stream << child;
    }
    indent = indent.substr(0, indent.size() - 2);
    return stream;
}
