#include "evaluator.h"
#include "optimizer.h"
#include "parser.h"
#include "tokenizer.h"

#include <cmath>
#include <iostream>

static expr::evaluator_result fn_sin(
    const std::vector<double>& parameters,
    const expr::location_t& location
) {
    if (parameters.size() != 1) {
        return expr::error{
            expr::error_code::EVALUATOR_WRONG_ARGUMENT_COUNT,
            location,
            "function sin(x) takes 1 argument"
        };
    }
    return sin(parameters[0]);
}

static expr::evaluator_result fn_log(
    const std::vector<double>& parameters,
    const expr::location_t& location
) {
    if (parameters.size() != 2) {
        return expr::error{
            expr::error_code::EVALUATOR_WRONG_ARGUMENT_COUNT,
            location,
            "function log(x, base) takes 2 arguments"
        };
    }
    return log(parameters[0]) / log(parameters[1]);
}

template <typename ProcessFn, typename... InputT>
auto process_and_print(
    const char *action,
    ProcessFn process,
    InputT&&... input
) {
    auto result = process(std::forward<InputT>(input)...);
    if (!result) {
        std::cout << "failed to " << action << ": "
                  << result.error().description
                  << '\n';
    }
    std::cout << *result << "\n\n";
    return std::move(result);
}

int main() {
    std::cout << "please enter an expression: ";
    std::string expression;
    std::getline(std::cin, expression);
    std::cout << "\n";

    using tokenize_string_fn = expr::tokenizer_result (*)(const std::string&);
    auto tokens = process_and_print<tokenize_string_fn>(
        "tokenize input",
        expr::tokenize,
        expression
    );
    if (!tokens)
        return 1;

    auto parsed = process_and_print(
        "parse tokens",
        expr::parse,
        std::move(*tokens)
    );
    if (!parsed)
        return 2;

    auto optimized = process_and_print(
        "optimize expression tree",
        expr::optimize,
        *parsed
    );
    if (!optimized)
        return 3;

    const auto symbols = expr::symbol_table{
        {"pi", 3.141592653589793238},
        {"e", 2.718281828459045235}
    };

    const auto functions = expr::function_table{
        {"sin", fn_sin},
        {"log", fn_log}
    };

    auto result = process_and_print(
        "evaluate expression tree",
        expr::evaluate,
        *optimized,
        symbols,
        functions
    );
    if (!result)
        return 4;

    return 0;
}
