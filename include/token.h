#if !defined(EXPRPARSER_TOKEN_HEADER)
#define EXPRPARSER_TOKEN_HEADER

#include <iosfwd>
#include <string>
#include <vector>

namespace expr {
    struct token_t final {
        enum class type_t {
            BOOLEAN,
            NUMBER,
            IDENTIFIER,
            PLUS,
            MINUS,
            SLASH,
            ASTERISK,
            CARET,
            OPENING_PARENTHESIS,
            CLOSING_PARENTHESIS,
            COMMA
        };

        type_t type;
        std::string content;
        size_t location;
    };

    using token_list = std::vector<token_t>;
}

std::ostream& operator<<(std::ostream& stream, expr::token_t::type_t kind);

std::ostream& operator<<(std::ostream& stream, const expr::token_t& token);

std::ostream& operator<<(std::ostream& stream, const expr::token_list& list);

#endif
