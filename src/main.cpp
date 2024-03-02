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

int main() {
    std::cout << "please enter an expression: ";
    std::string expression;
    std::getline(std::cin, expression);
    std::cout << "\n";

    auto tokens = expr::tokenize(expression);
    if (!tokens) {
        std::cout << "failed to tokenize input: "
                  << tokens.error().description
                  << '\n';
        return 1;
    }
    std::cout << *tokens << "\n\n";

    const auto tree = expr::parse(std::move(*tokens));
    if (!tree) {
        std::cout << "failed to parse tokens: "
                  << tree.error().description
                  << '\n';
    }
    std::cout << *tree << '\n';

    const auto final = expr::optimize(*tree);
    if (!final) {
        std::cout << "failed to optimize expression tree: "
                  << final.error().description
                  << '\n';
        return 1;
    }
    std::cout << *final << '\n';

    const auto symbols = expr::symbol_table{
        {"pi", 3.141592653589793238},
        {"e", 2.718281828459045235}
    };
    const auto functions = expr::function_table{
        {"sin", fn_sin},
        {"log", fn_log}
    };

    const auto result = expr::evaluate(*tree, symbols, functions);
    if (!result) {
        std::cout << "failed to evaluate expression tree: "
                  << result.error().description
                  << '\n';
        return 1;
    }
    std::cout << *result << "\n";

    return 0;
}
