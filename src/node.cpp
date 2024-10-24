#include "node.h"

#include <iostream>         // std::ostream

namespace expr {
    node_ptr make_boolean_literal_node(
        std::string content,
        location_t location
    ) {
        return std::unique_ptr<node_t>(
            new node_t{
                node_t::type_t::BOOLEAN,
                std::move(content),
                {},
                location
            }
        );
    }

    node_ptr make_number_literal_node(
        std::string content,
        location_t location
    ) {
        return std::unique_ptr<node_t>(
            new node_t{node_t::type_t::NUMBER, std::move(content), {}, location}
        );
    }

    node_ptr make_variable_node(std::string content, location_t location) {
        return std::unique_ptr<node_t>(
            new node_t{
                node_t::type_t::VARIABLE,
                std::move(content),
                {},
                location
            }
        );
    }

    node_ptr make_unary_operator_node(
        std::string content,
        node_ptr&& operand,
        location_t location
    ) {
        node_ptr result = std::unique_ptr<node_t>(
            new node_t{
                node_t::type_t::UNARY_OP,
                std::move(content),
                {},
                location
            }
        );
        result->children.push_back(std::move(operand));
        return result;
    }

    node_ptr make_binary_operator_node(
        std::string content,
        node_ptr&& left,
        node_ptr&& right,
        location_t location
    ) {
        node_ptr result = std::unique_ptr<node_t>(
            new node_t{
                node_t::type_t::BINARY_OP,
                std::move(content),
                {},
                location
            }
        );
        result->children.push_back(std::move(left));
        result->children.push_back(std::move(right));
        return result;
    }

    node_ptr make_function_call_node(
        std::string content,
        std::vector<node_ptr>&& parameters,
        location_t location
    ) {
        auto result = std::unique_ptr<node_t>(
            new node_t{
                node_t::type_t::FUNCTION_CALL,
                std::move(content),
                std::move(parameters),
                location
            }
        );
        return result;
    }
}

std::ostream& operator<<(std::ostream& stream, expr::node_t::type_t type) {
    switch (type) {
        case expr::node_t::type_t::BINARY_OP:
            return stream << "BinaryOperator";
        case expr::node_t::type_t::UNARY_OP:
            return stream << "UnaryOperator";
        case expr::node_t::type_t::NUMBER:
            return stream << "NumberLiteral";
        case expr::node_t::type_t::BOOLEAN:
            return stream << "BooleanLiteral";
        case expr::node_t::type_t::VARIABLE:
            return stream << "Variable";
        case expr::node_t::type_t::FUNCTION_CALL:
            return stream << "FunctionCall";
    }

    // Unreachable
    return stream;
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
