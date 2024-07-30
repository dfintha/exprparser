#include "tokenizer.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <optional>

static bool is_valid_numeric_part(const std::string& content, char current) {
    if (content == "0" && (current == 'x' || current == 'b'))
        return true;

    if (content.substr(0, 2) == "0b" && (current == '0' || current == '1'))
        return true;

    if (content.substr(0, 2) == "0x" && isxdigit(current))
        return true;

    const bool has_lowercase_e = content.find('e') != std::string::npos;
    const bool has_uppercase_e = content.find('E') != std::string::npos;
    const bool has_any_e = has_lowercase_e  || has_uppercase_e;

    if (!has_any_e) {
        if (current == 'e' || current == 'E')
            return true;

        if (content.find('.') == std::string::npos && current == '.')
            return true;
    }

    if (has_any_e && (current == '-' || current == '+'))
        return true;

    return isdigit(current);
}

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
        case '%':
            type = expr::token_t::type_t::PERCENT;
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
                } else if (isalpha(current) || current == '_') {
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
                if (is_valid_numeric_part(content, current)) {
                    content += current;
                } else if (current == '.') {
                    return expr::error{
                        expr::error_code::TOKENIZER_MULTIPLE_DECIMAL_DOT,
                        expr::location_t{i + 1, i + 1},
                        "Multiple decimal dots present in numeric literal."
                    };
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
            "Expression resulted in an empty token stream."
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

    tokenizer_result tokenize(std::string_view expression) {
        return ::tokenize(expression.data(), expression.length());
    }
}
