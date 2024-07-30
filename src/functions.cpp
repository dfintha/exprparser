#include "functions.h"

#include <cfloat>
#include <cmath>

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
                        "function " #NAME "(" ARGLIST ") takes "               \
                        #N_PARAMS " argument(s)"                               \
                    };                                                         \
                }                                                              \
                return IMPLEMENTATION;                                         \
            },                                                                 \
            #NAME "(" ARGLIST ")"                                              \
        }                                                                      \
    }

const expr::function_table& expr::functions() {
    static const auto table = expr::function_table{
        EXPR_BUILTIN_FUNCTION(sin, 1, sin(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(cos, 1, cos(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(tan, 1, tan(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(ctg, 1, 1.0 / tan(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(sec, 1, 1.0 / cos(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(csc, 1, 1.0 / sin(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(round, 1, round(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(floor, 1, floor(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(ceil, 1, ceil(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(abs, 1, fabs(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(ln, 1, log(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(log2, 1, log2(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(log10, 1, log10(params[0]), "x"),
        EXPR_BUILTIN_FUNCTION(log, 2, log_n(params[0], params[1]), "x, base"),
        EXPR_BUILTIN_FUNCTION(sgn, 1, sgn(params[0]), "x"),
    };
    return table;
}

#undef EXPR_BUILTIN_FUNCTION
