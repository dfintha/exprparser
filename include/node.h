#if !defined(EXPRPARSER_NODE_HEADER)
#define EXPRPARSER_NODE_HEADER

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace expr {
    struct node_t;

    using node_ptr = std::unique_ptr<node_t>;

    struct node_t final {
        enum class type_t {
            BINARY_OP,
            UNARY_OP,
            NUMBER,
            BOOLEAN,
            IDENTIFIER,
            FUNCTION_CALL,
        };

        type_t type;
        std::string content;
        std::vector<node_ptr> children;
    };

    node_ptr make_boolean_literal_node(std::string content);

    node_ptr make_number_literal_node(std::string content);

    node_ptr make_identifier_node(std::string content);

    node_ptr make_unary_operator_node(std::string content, node_ptr&& operand);

    node_ptr make_binary_operator_node(
        std::string content,
        node_ptr&& left,
        node_ptr&& right
    );

    node_ptr make_function_call_node(
        std::string content,
        std::vector<node_ptr>&& parameters
    );
}

std::ostream& operator<<(std::ostream& stream, expr::node_t::type_t kind);

std::ostream& operator<<(std::ostream& stream, const expr::node_ptr& token);

#endif
