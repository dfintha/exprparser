#include "functions.h"

#include <cfloat>
#include <cmath>

#define EXPR_DEFINE_BUILTIN_FUNCTION(NAME, N_PARAM, IMPLEMENTATION, SIGNATURE) \
    static expr::function_result fn_ ## NAME(                                  \
        const std::vector<double>& parameters,                                 \
        const expr::location_t& location                                       \
    ) {                                                                        \
        if (parameters.size() != N_PARAM) {                                    \
            return expr::error{                                                \
                expr::error_code::EVALUATOR_WRONG_ARGUMENT_COUNT,              \
                location,                                                      \
                "function " #NAME "(" SIGNATURE ") takes "                     \
                #N_PARAM " argument(s)"                                        \
            };                                                                 \
        }                                                                      \
        return IMPLEMENTATION;                                                 \
    }

EXPR_DEFINE_BUILTIN_FUNCTION(sin, 1, sin(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(cos, 1, cos(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(tan, 1, tan(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(ctg, 1, 1.0 / tan(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(sec, 1, 1.0 / cos(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(csc, 1, 1.0 / sin(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(round, 1, round(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(floor, 1, floor(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(ceil, 1, ceil(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(abs, 1, fabs(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(ln, 1, log(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(log2, 1, log2(parameters[0]), "x");
EXPR_DEFINE_BUILTIN_FUNCTION(log10, 1, log10(parameters[0]), "x");

EXPR_DEFINE_BUILTIN_FUNCTION(
    log,
    2,
    log(parameters[0]) / log(parameters[1]),
    "x, base"
);

EXPR_DEFINE_BUILTIN_FUNCTION(
    sgn,
    1,
    ((fabs(parameters[0]) < DBL_EPSILON)
        ? 0.0
        : ((parameters[0] < 0) ? -1.0 : 1.0)),
    "x"
);

#undef EXPR_DEFINE_BUILTIN_FUNCTION

const expr::function_table& expr::functions() {
    static const auto table = expr::function_table{
        {"sin", fn_sin},
        {"cos", fn_cos},
        {"tan", fn_tan},
        {"ctg", fn_ctg},
        {"sec", fn_sec},
        {"csc", fn_csc},
        {"round", fn_round},
        {"floor", fn_floor},
        {"ceil", fn_ceil},
        {"abs", fn_abs},
        {"ln", fn_ln},
        {"log2", fn_log2},
        {"log10", fn_log10},
        {"log", fn_log},
        {"sgn", fn_sgn},
    };
    return table;
}
