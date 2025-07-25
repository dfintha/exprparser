#include "token.h"

#include <iostream>         // std::ostream

std::ostream& operator<<(std::ostream& stream, expr::token_t::type_t type) {
    switch (type) {
        case expr::token_t::type_t::NUMBER:
            return stream << "NumberLiteral";
        case expr::token_t::type_t::IDENTIFIER:
            return stream << "Identifier";
        case expr::token_t::type_t::PLUS:
            return stream << "Plus";
        case expr::token_t::type_t::MINUS:
            return stream << "Minus";
        case expr::token_t::type_t::SLASH:
            return stream << "Slash";
        case expr::token_t::type_t::ASTERISK:
            return stream << "Asterisk";
        case expr::token_t::type_t::PERCENT:
            return stream << "Percent";
        case expr::token_t::type_t::CARET:
            return stream << "Caret";
        case expr::token_t::type_t::OPENING_PARENTHESIS:
            return stream << "OpeningParenthesis";
        case expr::token_t::type_t::CLOSING_PARENTHESIS:
            return stream << "ClosingParenthesis";
        case expr::token_t::type_t::COMMA:
            return stream << "Comma";
        case expr::token_t::type_t::EQUAL_SIGN:
            return stream << "EqualSign";
        case expr::token_t::type_t::UNIT:
            return stream << "Unit";
    }

    // Unreachable
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const expr::token_t& token) {
    return stream << token.type
                  << "('" << token.content
                  << "'@" << token.location << ')';
}

std::ostream& operator<<(std::ostream& stream, const expr::token_list& list) {
    if (list.size() == 0)
        return stream << "[]";

    stream << "[" << list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        stream << ",\n " << list[i];
    }

    return stream << "]\n";
}
