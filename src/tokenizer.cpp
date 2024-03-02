#include "tokenizer.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <optional>

static std::optional<expr::token_t> extract_single(
    char current,
    size_t location
) {
    expr::token_t::type_t type;
    switch (current) {
        case '+':
            type = expr::token_t::type_t::PLUS;
            break;
        case '-':
            type = expr::token_t::type_t::MINUS;
            break;
        case '*':
            type = expr::token_t::type_t::ASTERISK;
            break;
        case '/':
            type = expr::token_t::type_t::SLASH;
            break;
        case '^':
            type = expr::token_t::type_t::CARET;
            break;
        case '(':
            type = expr::token_t::type_t::OPENING_PARENTHESIS;
            break;
        case ')':
            type = expr::token_t::type_t::CLOSING_PARENTHESIS;
            break;
        case ',':
            type = expr::token_t::type_t::COMMA;
            break;
        default:
            return std::nullopt;
    }

    return expr::token_t{type, std::string{current}, {location, location + 1}};
}

static expr::token_t extract_word(std::string&& content, size_t location) {
    static const std::vector<std::string> booleans = { "true", "false" };
    const auto lookup = std::find(booleans.begin(), booleans.end(), content);
    const bool match = lookup != booleans.end();
    const size_t length = content.length();
    return expr::token_t{
        match ? expr::token_t::type_t::BOOLEAN
              : expr::token_t::type_t::IDENTIFIER,
        std::move(content),
        {location, location + length}
    };
}

static expr::tokenizer_result tokenize(const char *expression, size_t length) {
    enum class state_t {
        NORMAL,
        IN_WORD,
        IN_NUMBER,
    };

    expr::token_list result;
    state_t state = state_t::NORMAL;
    std::string content;
    size_t location = 0;

    for (size_t i = 0; i < (length + 1); ++i) {
        const char current = expression[i];
        switch (state) {
            case state_t::NORMAL: {
                if (auto token = extract_single(current, i + 1)) {
                    result.push_back(*token);
                } else if (isalpha(current)) {
                    state = state_t::IN_WORD;
                    content = std::string{current};
                    location = i + 1;
                } else if (isdigit(current)) {
                    state = state_t::IN_NUMBER;
                    content = std::string{current};
                    location = i + 1;
                }
                break;
            }
            case state_t::IN_WORD: {
                if (isalnum(current) || current == '_') {
                    content += current;
                } else {
                    result.push_back(
                        extract_word(std::move(content), location)
                    );
                    state = state_t::NORMAL;
                    --i;
                }
                break;
            }
            case state_t::IN_NUMBER: {
                const bool decimal = (content.find('.') != std::string::npos);
                if (isdigit(current) || (current == '.' && !decimal)) {
                    content += current;
                } else {
                    result.push_back(expr::token_t{
                        expr::token_t::type_t::NUMBER,
                        content,
                        {location, location + content.length()}
                    });
                    state = state_t::NORMAL;
                    --i;
                }
                break;
            }
        }
    }

    if (result.size() == 0) {
        return expr::error{
            expr::error_code::TOKENIZER_EMPTY_INPUT,
            expr::location_t{0, 0},
            "expression resulted in an empty token stream"
        };
    }

    return result;
}

namespace expr {
    tokenizer_result tokenize(const char *expression) {
        return ::tokenize(expression, strlen(expression));
    }

    tokenizer_result tokenize(const std::string& expression) {
        return ::tokenize(expression.c_str(), expression.length());
    }
}
