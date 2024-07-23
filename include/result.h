#if !defined(EXPRPARSER_RESULT_HEADER)
#define EXPRPARSER_RESULT_HEADER

#include "location.h"

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace expr {
    enum error_code : unsigned {
        TOKENIZER_CODES_BEGIN = 1000,
        TOKENIZER_EMPTY_INPUT = 1001,
        TOKENIZER_MULTIPLE_DECIMAL_DOT = 1002,

        PARSER_CODES_BEGIN = 2000,
        PARSER_GENERAL_ERROR = 2001,

        OPTIMIZER_CODES_BEGIN = 3000,
        OPTIMIZER_FAILED_TO_OPTIMIZE_CHILD = 3001,

        EVALUATOR_CODES_BEGIN = 4000,
        EVALUATOR_FAILED_TO_EVALUATE_OPERAND = 4001,
        EVALUATOR_UNDEFINED_VARIABLE = 4002,
        EVALUATOR_UNDEFINED_FUNCTION = 4003,
        EVALUATOR_FAILED_TO_EVALUATE_ARGUMENTS = 4004,
        EVALUATOR_WRONG_ARGUMENT_COUNT = 4005,
    };

    struct error {
        error_code code;
        expr::location_t location;
        std::string description;
    };

    template <typename Expected, typename Error>
    class result {
    public:
        using value_type = Expected;
        using error_type = Error;

        static_assert(!std::is_convertible_v<value_type, error_type>);

    public:
        template <typename Other>
        result(Other&& other) : contents(std::forward<Other>(other)) {
        }

        bool has_value() const {
            return value_ptr() != nullptr;
        }

        bool has_error() const {
            return error_ptr() != nullptr;
        }

        operator bool() const {
            return has_value();
        }

        value_type& value() {
            return *value_ptr();
        }

        const value_type& value() const {
            return *value_ptr();
        }

        error_type& error() {
            return *error_ptr();
        }

        const error_type& error() const {
            return *error_ptr();
        }

        value_type& operator*() {
            return value();
        }

        const value_type& operator*() const {
            return value();
        }

        value_type * operator->() {
            return value_ptr();
        }

        const value_type * operator->() const {
            return value_ptr();
        }

    private:
        const value_type * value_ptr() const {
            return std::get_if<value_type>(&contents);
        }

        value_type * value_ptr() {
            return std::get_if<value_type>(&contents);
        }

        const error_type * error_ptr() const {
            return std::get_if<error_type>(&contents);
        }

        error_type * error_ptr() {
            return std::get_if<error_type>(&contents);
        }

    private:
        std::variant<value_type, error_type> contents;
    };
}

#endif
