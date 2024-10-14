#include "functions.h"

#include <cfloat>           // DBL_EPSILON
#include <cmath>            // all math functions

static double log_n(double x, double base) {
    return log10(x) / log10(base);
}

static double sgn(double x) {
    return (fabs(x) < DBL_EPSILON) ? 0.0 : ((x < 0) ? -1.0 : 1.0);
}

#define EXPR_BUILTIN_FUNCTION(NAME, N_PARAMS, IMPLEMENTATION, ARGLIST)         \
    {                                                                          \
        std::string(#NAME),                                                    \
        {                                                                      \
            [](const std::vector<double>& params,                              \
               const expr::location_t& location                                \
            ) -> expr::function_result {                                       \
                if (params.size() != N_PARAMS) {                               \
                    return expr::error{                                        \
                        expr::error_code::EVALUATOR_WRONG_ARGUMENT_COUNT,      \
                        location,                                              \
                        "Function " #NAME "(" ARGLIST ") takes "               \
                        #N_PARAMS " argument(s)."                              \
                    };                                                         \
                }                                                              \
                return IMPLEMENTATION;                                         \
            },                                                                 \
            #NAME "(" ARGLIST ")"                                              \
        }                                                                      \
    }

const expr::function_table& expr::functions() {
    static const auto table = expr::function_table{
        EXPR_BUILTIN_FUNCTION(sin, 1, std::sin(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(cos, 1, std::cos(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(tan, 1, std::tan(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(ctg, 1, 1.0 / std::tan(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(sec, 1, 1.0 / std::cos(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(csc, 1, 1.0 / std::sin(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(round, 1, std::round(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(floor, 1, std::floor(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(ceil, 1, std::ceil(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(abs, 1, std::fabs(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(ln, 1, std::log(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(log2, 1, std::log2(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(log10, 1, std::log10(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(log, 2, log_n(params[0], params[1]), "x, base"),
        EXPR_BUILTIN_FUNCTION(sgn, 1, sgn(params[0]), "x"),
    };
    return table;
}

#undef EXPR_BUILTIN_FUNCTION
