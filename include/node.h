#if !defined(EXPRPARSER_NODE_HEADER)
#define EXPRPARSER_NODE_HEADER

#include "location.h"

#include <iosfwd>           // std::ostream
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
            BOOLEAN,
            VARIABLE,
            FUNCTION_CALL,
        };

        type_t type;
        std::string content;
        std::vector<node_ptr> children;
        location_t location;
    };

    node_ptr make_boolean_literal_node(
        std::string content,
        location_t location
    );

    node_ptr make_number_literal_node(std::string content, location_t location);

    node_ptr make_variable_node(std::string content, location_t location);

    node_ptr make_unary_operator_node(
        std::string content,
        node_ptr&& operand,
        location_t location);

    node_ptr make_binary_operator_node(
        std::string content,
        node_ptr&& left,
        node_ptr&& right,
        location_t location
    );

    node_ptr make_function_call_node(
        std::string content,
        std::vector<node_ptr>&& parameters,
        location_t location
    );
}

std::ostream& operator<<(std::ostream& stream, expr::node_t::type_t kind);

std::ostream& operator<<(std::ostream& stream, const expr::node_ptr& token);

#endif
