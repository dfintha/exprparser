#if !defined(EXPRPARSER_TOKEN_HEADER)
#define EXPRPARSER_TOKEN_HEADER

#include "location.h"

#include <iosfwd>           // std::ostream
#include <string>           // std::string
#include <vector>           // std::vector

namespace expr {
    struct token_t final {
        enum class type_t {
            NUMBER,
            IDENTIFIER,
            PLUS,
            MINUS,
            SLASH,
            ASTERISK,
            PERCENT,
            CARET,
            OPENING_PARENTHESIS,
            CLOSING_PARENTHESIS,
            COMMA,
            EQUALS
        };

        type_t type;
        std::string content;
        location_t location;
    };

    using token_list = std::vector<token_t>;
}

std::ostream& operator<<(std::ostream& stream, expr::token_t::type_t kind);
std::ostream& operator<<(std::ostream& stream, const expr::token_t& token);
std::ostream& operator<<(std::ostream& stream, const expr::token_list& list);

#endif
