#include "tokenizer.h"

#include <algorithm>        // std::find
#include <cctype>           // std::isalpha, std::isdigit, std::isxdigit
#include <cstring>          // std::strlen
#include <optional>         // std::optional, std::nullopt
#include <unordered_set>    // std::unordered_set

static bool is_valid_numeric_part(
    const std::string& content,
    char current
) noexcept {
    if (content == "0" && (current == 'x' || current == 'b'))
        return true;

    if (content.substr(0, 2) == "0b" && (current == '0' || current == '1'))
        return true;

    if (content.substr(0, 2) == "0x" && std::isxdigit(current))
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

    return std::isdigit(current);
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
        case '=':
            type = expr::token_t::type_t::EQUAL_SIGN;
            break;
        default:
            return std::nullopt;
    }

    return expr::token_t{type, std::string{current}, {location, location + 1}};
}

static expr::token_t extract_word(
    std::string&& content,
    size_t location
) {
    static const std::unordered_set<std::string_view> units = {
        "mm", "cm", "m", "km", "rad", "deg"
    };

    auto type = units.contains(content) ? expr::token_t::type_t::UNIT
                                        : expr::token_t::type_t::IDENTIFIER;
    const size_t length = content.length();
    return expr::token_t{
        .type = type,
        .content = std::move(content),
        .location = {location, location + length}
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

expr::tokenizer_result expr::tokenize(const char *expression) {
    return ::tokenize(expression, std::strlen(expression));
}

expr::tokenizer_result expr::tokenize(const std::string& expression) {
    return ::tokenize(expression.c_str(), expression.length());
}

expr::tokenizer_result expr::tokenize(std::string_view expression) {
    return ::tokenize(expression.data(), expression.length());
}
