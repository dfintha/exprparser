#include "parser.h"

#include <iostream>
#include <unordered_set>    // std::unordered_set

class expression_parser_impl final {
public:
    expression_parser_impl(expr::token_list&& tokens);
    expr::parser_result parse();
    expr::location_t get_source_range() const;

private:
    expr::parser_result parse_assignment();
    expr::parser_result parse_function_call();
    expr::parser_result parse_primary();
    expr::parser_result parse_unary();
    expr::parser_result parse_power();
    expr::parser_result parse_factor();
    expr::parser_result parse_term();

private:
    using token_type_t = expr::token_t::type_t;

private:
    const expr::token_t& previous() {
        return *(_position - 1);
    }

    bool match(const std::unordered_set<token_type_t>& types) {
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

expr::parser_result expression_parser_impl::parse_function_call() {
    const auto begin = previous().location.begin;
    auto name = previous().content;
    size_t end;
    std::vector<expr::node_ptr> parameters;

    if (_position != _tokens.end())
        ++_position;

    while (true) {
        auto parameter = parse_term();
        if (!parameter)
            return parameter;
        parameters.push_back(std::move(*parameter));

        const auto& type = _position->type;
        const bool is_comma = type == token_type_t::COMMA;
        const bool is_closing = type == token_type_t::CLOSING_PARENTHESIS;

        if (is_closing)
            end = _position->location.end;

        if (!is_comma && !is_closing)
            return expr::error{
                expr::error_code::PARSER_UNEXPECTED_TOKEN,
                _position->location,
                "Unexpected token."
            };

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

expr::parser_result expression_parser_impl::parse_primary() {
    if (match({token_type_t::BOOLEAN})) {
        return expr::make_number_literal_node(
            previous().content == "true" ? "1" : "0",
            previous().location
        );
    }

    if (match({token_type_t::NUMBER})) {
        return expr::make_number_literal_node(
            previous().content,
            previous().location
        );
    }

    if (match({token_type_t::IDENTIFIER})) {
        if (_position->type != token_type_t::OPENING_PARENTHESIS) {
            return expr::make_variable_node(
                previous().content,
                previous().location
            );
        }
        return parse_function_call();
    }

    if (match({token_type_t::OPENING_PARENTHESIS})) {
        const auto begin = previous().location.begin;

        auto subexpression = parse_term();
        if (!subexpression)
            return subexpression;

        if (_position->type == token_type_t::CLOSING_PARENTHESIS) {
            if (_position != _tokens.end())
                ++_position;
            return std::move(*subexpression);
        } else if (_position == _tokens.end()) {
            return expr::error {
                expr::error_code::PARSER_UNCLOSED_PARENTHESES,
                expr::location_t{begin, begin},
                "Unclosed parenthesis."
            };
        }
    }

    return expr::error {
        expr::error_code::PARSER_UNEXPECTED_TOKEN,
        _position->location,
        "Unexpected token '" + _position->content + "'."
    };
}

expr::parser_result expression_parser_impl::parse_unary() {
    if (match({token_type_t::PLUS, token_type_t::MINUS})) {
        const auto begin = previous().location.begin;

        auto subexpression = parse_unary();
        if (!subexpression)
            return subexpression;

        return expr::make_unary_operator_node(
            previous().content,
            std::move(*subexpression),
            expr::location_t{begin, previous().location.end}
        );
    }
    return parse_primary();
}

expr::parser_result expression_parser_impl::parse_power() {
    auto lhs = parse_unary();
    if (!lhs)
        return lhs;

    expr::node_ptr expression = std::move(*lhs);
    while (match({token_type_t::CARET})) {
        const auto begin = expression->location.begin;
        auto content = previous().content;

        auto rhs = parse_unary();
        if (!rhs)
            return rhs;

        expression = expr::make_binary_operator_node(
            content,
            std::move(expression),
            std::move(*rhs),
            expr::location_t{begin, previous().location.end}
        );
    }

    return expression;
}

expr::parser_result expression_parser_impl::parse_factor() {
    static const std::unordered_set<token_type_t> tokens = {
        token_type_t::ASTERISK,
        token_type_t::SLASH,
        token_type_t::PERCENT
    };

    auto lhs = parse_power();
    if (!lhs)
        return lhs;

    expr::node_ptr expression = std::move(*lhs);
    while (match(tokens)) {
        const auto begin = expression->location.begin;
        auto content = previous().content;

        auto rhs = parse_power();
        if (!rhs)
            return rhs;

        expression = expr::make_binary_operator_node(
            content,
            std::move(expression),
            std::move(*rhs),
            expr::location_t{begin, previous().location.end}
        );
    }

    return expression;
}

expr::parser_result expression_parser_impl::parse_term() {
    auto lhs = parse_factor();
    if (!lhs)
        return lhs;

    expr::node_ptr expression = std::move(*lhs);
    while (match({token_type_t::PLUS, token_type_t::MINUS})) {
        const auto begin = expression->location.begin;
        auto content = previous().content;

        auto rhs = parse_factor();
        if (!rhs)
            return rhs;

        expression = make_binary_operator_node(
            content,
            std::move(expression),
            std::move(*rhs),
            expr::location_t{begin, previous().location.end}
        );
    }

    return expression;
}

expr::parser_result expression_parser_impl::parse_assignment() {
    auto lhs = parse_term();
    if (!lhs)
        return lhs;

    expr::node_ptr expression = std::move(*lhs);
    while (match({token_type_t::EQUALS})) {
        const auto begin = expression->location.begin;
        auto content = previous().content;

        if (expression->type != expr::node_t::type_t::VARIABLE) {
            return expr::error {
                expr::error_code::PARSER_NON_VARIABLE_ASSIGNMENT,
                _position->location,
                "Only variables can be assigned."
            };
        }

        auto rhs = parse_term();
        if (!rhs)
            return rhs;

        expression = expr::make_assignment_node(
            std::move(expression),
            std::move(*rhs),
            expr::location_t{begin, previous().location.end}
        );
    }

    return expression;
}

expr::parser_result expression_parser_impl::parse() {
    auto result = parse_assignment();
    if (!result)
        return result;

    if (_position != _tokens.end()) {
        return expr::error {
            expr::error_code::PARSER_PARTIAL_PARSE,
            expr::location_t{0, previous().location.end - 1},
            "Token list was only partially parsed. "
            "Extraneous parentheses or missing operands?"
        };
    }

    return result;
}

expr::location_t expression_parser_impl::get_source_range() const {
    return expr::location_t{
        _tokens.front().location.begin,
        _tokens.back().location.end
    };
}

namespace expr {
    parser_result parse(token_list&& tokens) {
        auto parser = expression_parser_impl(std::move(tokens));
        auto result = parser.parse();

        if (!result)
            return result;

        if (*result != nullptr)
            return std::move(*result);

        return expr::error {
            expr::error_code::PARSER_GENERAL_ERROR,
            parser.get_source_range(),
            "Unknown error occurred during token list parsing."
        };
    }
}
