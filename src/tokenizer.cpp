#include "tokenizer.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <optional>

static std::optional<expr::token_t> extract_single(char current) {
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
    return expr::token_t{type, std::string{current}};
}

static expr::token_t extract_word(std::string&& content) {
    static const std::vector<std::string> booleans = { "true", "false" };
    const auto lookup = std::find(booleans.begin(), booleans.end(), content);
    const bool match = lookup != booleans.end();
    return expr::token_t{
        match ? expr::token_t::type_t::BOOLEAN
              : expr::token_t::type_t::IDENTIFIER,
        content
    };
}

static expr::token_list tokenize(const char *expression, size_t length) {
    enum class state_t {
        NORMAL,
        IN_WORD,
        IN_NUMBER,
    };

    expr::token_list result;
    state_t state = state_t::NORMAL;
    std::string content;

    for (size_t i = 0; i < (length + 1); ++i) {
        const char current = expression[i];
        switch (state) {
            case state_t::NORMAL: {
                if (auto token = extract_single(current)) {
                    result.push_back(*token);
                } else if (isalpha(current)) {
                    state = state_t::IN_WORD;
                    content = std::string{current};
                } else if (isdigit(current)) {
                    state = state_t::IN_NUMBER;
                    content = std::string{current};
                }
                break;
            }
            case state_t::IN_WORD: {
                if (isalnum(current) || current == '_') {
                    content += current;
                } else {
                    result.push_back(extract_word(std::move(content)));
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
                        content
                    });
                    state = state_t::NORMAL;
                    --i;
                }
                break;
            }
        }
    }

    return result;
}

namespace expr {
    token_list tokenize(const char *expression) {
        return ::tokenize(expression, strlen(expression));
    }

    token_list tokenize(const std::string& expression) {
        return ::tokenize(expression.c_str(), expression.length());
    }
}
