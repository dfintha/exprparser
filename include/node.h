#if !defined(EXPRPARSER_NODE_HEADER)
#define EXPRPARSER_NODE_HEADER

#include "location.h"

#include <memory>           // std::unique_ptr
#include <string>           // std::string
#include <vector>           // std::vector

namespace expr {
    struct node_t;

    using node_ptr = std::unique_ptr<node_t>;

    struct node_t final {
        enum class type_t {
            BINARY_OP,
            UNARY_OP,
            NUMBER,
            VARIABLE,
            FUNCTION_CALL,
            ASSIGNMENT,
            UNIT,
            UNIT_APPLICATION,
        };

        type_t type;
        std::string content;
        std::vector<node_ptr> children;
        location_t location;

        friend bool operator==(const node_t& lhs, const node_t& rhs) noexcept;
        friend bool operator!=(const node_t& lhs, const node_t& rhs) noexcept;
    };

    node_ptr make_number_literal_node(
        std::string content,
        const location_t& location
    );

    node_ptr make_variable_node(
        std::string content,
        const location_t& location
    );

    node_ptr make_unary_operator_node(
        std::string content,
        node_ptr&& operand,
        const location_t& location);

    node_ptr make_binary_operator_node(
        std::string content,
        node_ptr&& left,
        node_ptr&& right,
        const location_t& location
    );

    node_ptr make_function_call_node(
        std::string content,
        std::vector<node_ptr>&& parameters,
        const location_t& location
    );

    node_ptr make_assignment_node(
        node_ptr&& left,
        node_ptr&& right,
        const location_t& location
    );

    node_ptr make_unit_node(
        std::string content,
        const location_t& location
    );

    node_ptr make_unit_application_node(
        node_ptr&& subexpression,
        node_ptr&& unit,
        const location_t& location
    );

    std::string to_expression_string(const node_ptr& root);
}

std::ostream& operator<<(std::ostream& stream, expr::node_t::type_t kind);
std::ostream& operator<<(std::ostream& stream, const expr::node_ptr& token);

#endif
