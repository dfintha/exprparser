#include "parser.h"

#include <unordered_set>

class expression_parser_impl final {
public:
    expression_parser_impl(expr::token_list&& tokens);
    expr::node_ptr parse();

private:
    expr::node_ptr parse_function_call();
    expr::node_ptr parse_primary();
    expr::node_ptr parse_unary();
    expr::node_ptr parse_power();
    expr::node_ptr parse_factor();
    expr::node_ptr parse_term();

private:
    using token_type_t = expr::token_t::type_t;

private:
    const expr::token_t& previous() {
        return *(_position - 1);
    }

    bool match(std::unordered_set<token_type_t>&& types) {
        if (_position == _tokens.end()) {
            return false;
        }
        for (const auto type : types) {
            if (_position->type == type) {
                if (_position != _tokens.end())
                    ++_position;
                return true;
            }
        }
        return false;
    }

private:
    const expr::token_list _tokens;
    expr::token_list::const_iterator _position;
};

expression_parser_impl::expression_parser_impl(expr::token_list&& tokens) :
    _tokens(std::move(tokens)),
    _position(_tokens.begin())
{}

expr::node_ptr expression_parser_impl::parse_function_call() {
    const size_t begin = previous().location.begin;
    size_t end;
    std::string name = previous().content;
    std::vector<expr::node_ptr> parameters;

    if (_position != _tokens.end())
        ++_position;

    while (true) {
        parameters.push_back(parse_term());

        const auto& type = _position->type;
        const bool is_comma = type == token_type_t::COMMA;
        const bool is_closing = type == token_type_t::CLOSING_PARENTHESIS;

        if (is_closing)
            end = _position->location.end;

        if (!is_comma && !is_closing)
            return nullptr;

        if (_position != _tokens.end())
            ++_position;

        if (is_closing)
            break;
    }

    return expr::make_function_call_node(
        std::move(name),
        std::move(parameters),
        expr::location_t{begin, end}
    );
}

expr::node_ptr expression_parser_impl::parse_primary() {
    if (match({token_type_t::BOOLEAN}))
        return expr::make_boolean_literal_node(
            previous().content,
            previous().location
        );

    if (match({token_type_t::NUMBER}))
        return expr::make_number_literal_node(
            previous().content,
            previous().location
        );

    if (match({token_type_t::IDENTIFIER})) {
        if (_position->type != token_type_t::OPENING_PARENTHESIS)
            return expr::make_variable_node(
                previous().content,
                previous().location
            );
        return parse_function_call();
    }

    if (match({token_type_t::OPENING_PARENTHESIS})) {
        auto subexpression = parse_term();
        if (_position->type == token_type_t::CLOSING_PARENTHESIS) {
            if (_position != _tokens.end())
                ++_position;
            return subexpression;
        }
    }

    return nullptr;
}

expr::node_ptr expression_parser_impl::parse_unary() {
    if (match({token_type_t::PLUS, token_type_t::MINUS})) {
        const size_t begin = previous().location.begin;
        return expr::make_unary_operator_node(
            previous().content,
            parse_unary(),
            expr::location_t{begin, previous().location.end}
        );
    }
    return parse_primary();
}

expr::node_ptr expression_parser_impl::parse_power() {
    expr::node_ptr expression = parse_unary();
    while (match({token_type_t::CARET})) {
        const size_t begin = expression->location.begin;
        expression = expr::make_binary_operator_node(
            previous().content,
            std::move(expression),
            parse_unary(),
            expr::location_t{begin, previous().location.end}
        );
    }
    return expression;
}

expr::node_ptr expression_parser_impl::parse_factor() {
    expr::node_ptr expression = parse_power();
    while (match({token_type_t::ASTERISK, token_type_t::SLASH})) {
        const size_t begin = expression->location.begin;
        expression = expr::make_binary_operator_node(
            previous().content,
            std::move(expression),
            parse_power(),
            expr::location_t{begin, previous().location.end}
        );
    }
    return expression;
}

expr::node_ptr expression_parser_impl::parse_term() {
    expr::node_ptr expression = parse_factor();
    while (match({token_type_t::PLUS, token_type_t::MINUS})) {
        const size_t begin = expression->location.begin;
        expression = make_binary_operator_node(
            previous().content,
            std::move(expression),
            parse_factor(),
            expr::location_t{begin, previous().location.end}
        );
    }
    return expression;
}

expr::node_ptr expression_parser_impl::parse() {
    return parse_term();
}

namespace expr {
    parser_result parse(token_list&& tokens) {
        expression_parser_impl parser(std::move(tokens));
        return parser.parse();
    }
}
