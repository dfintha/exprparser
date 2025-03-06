#include "functions.h"
#include "quantity.h"

#include <cfloat>           // DBL_EPSILON
#include <cmath>            // all math functions

#define EXPR_BUILTIN_FUNCTION(NAME)                                            \
    static expr::function_result NAME(                                         \
        const std::vector<expr::quantity>& parameters,                         \
        const expr::location_t& location                                       \
    ) noexcept

#define EXPR_CHECK_PARAMETER_COUNT(N)                                          \
    if (parameters.size() != N) {                                              \
        return expr::error{                                                    \
            expr::error_code::EVALUATOR_WRONG_ARGUMENT_COUNT,                  \
            location,                                                          \
            #N " argument(s) expected."                                        \
        };                                                                     \
    }

#define EXPR_CHECK_PARAMETER_TYPE(N, TYPE)                                     \
    if (!parameters[N].is_ ## TYPE ()) {                                       \
        return expr::error{                                                    \
            expr::error_code::EVALUATOR_WRONG_ARGUMENT_TYPE,                   \
            location,                                                          \
            "Argument at position " #N " is expected to be a " #TYPE "."       \
        };                                                                     \
    }

EXPR_BUILTIN_FUNCTION(sine) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, angle);
    return expr::make_scalar(std::sin(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(cosine) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, angle);
    return expr::make_scalar(std::cos(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(round) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    return expr::make_scalar(std::round(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(floor) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    return expr::make_scalar(std::floor(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(ceiling) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    return expr::make_scalar(std::ceil(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(absolute) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    auto result = parameters[0];
    result.value = std::abs(result.value);
    return result;
}

EXPR_BUILTIN_FUNCTION(log_n) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    return expr::make_scalar(std::log(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(log_2) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    return expr::make_scalar(std::log2(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(log_10) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    return expr::make_scalar(std::log10(parameters[0].value));
}

EXPR_BUILTIN_FUNCTION(log_any) {
    EXPR_CHECK_PARAMETER_COUNT(2);
    EXPR_CHECK_PARAMETER_TYPE(0, scalar);
    EXPR_CHECK_PARAMETER_TYPE(1, scalar);
    const auto& value = parameters[0].value;
    const auto& base = parameters[1].value;
    return expr::make_scalar(std::log10(value) / std::log10(base));
}

EXPR_BUILTIN_FUNCTION(sign) {
    EXPR_CHECK_PARAMETER_COUNT(1);
    if (std::fabs(parameters[0].value) < DBL_EPSILON)
        return expr::make_scalar(0);
    if (parameters[0].value < 0)
        return expr::make_scalar(-1);
    return expr::make_scalar(1);
}

const expr::function_table& expr::functions() {
    static const auto table = expr::function_table{
        {std::string{"sin"}, expr::function_definition_t{sine, "sin(x: angle) -> scalar"}},
        {std::string{"cos"}, expr::function_definition_t{cosine, "cos(x: angle) -> scalar"}},
        // {std::string{"tan"}, expr::function_definition_t{tangent, "tan(x: angle) -> scalar"}},
        // {std::string{"ctg"}, expr::function_definition_t{cotangent, "ctg(x: angle) -> scalar"}},
        // {std::string{"sec"}, expr::function_definition_t{secant, "sec(x: angle) -> scalar"}},
        // {std::string{"csc"}, expr::function_definition_t{cosecant, "csc(x: angle) -> scalar"}},
        {std::string{"round"}, expr::function_definition_t{round, "round(x: scalar) -> scalar"}},
        {std::string{"floor"}, expr::function_definition_t{floor, "floor(x: scalar) -> scalar"}},
        {std::string{"ceil"}, expr::function_definition_t{ceiling, "ceil(x: scalar) -> scalar"}},
        {std::string{"abs"}, expr::function_definition_t{absolute, "abs(x: any) -> any"}},
        {std::string{"ln"}, expr::function_definition_t{log_n, "ln(x: scalar) -> scalar"}},
        {std::string{"log2"}, expr::function_definition_t{log_2, "log2(x: scalar) -> scalar"}},
        {std::string{"log10"}, expr::function_definition_t{log_10, "log10(x: scalar) -> scalar"}},
        {std::string{"log"}, expr::function_definition_t{log_any, "log(x: scalar, base: scalar) -> scalar"}},
        {std::string{"sgn"}, expr::function_definition_t{sign, "sgn(x: any) -> scalar"}},
    };
    return table;
}

#undef EXPR_BUILTIN_FUNCTION
#undef EXPR_CHECK_PARAMETER_COUNT
#undef EXPR_CHECK_PARAMETER_TYPE
