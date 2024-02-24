#include "evaluator.h"
#include "parser.h"
#include "tokenizer.h"

#include <cmath>
#include <iostream>

// sin(x)
static std::optional<double> fn_sin(const std::vector<double>& parameters) {
    if (parameters.size() != 1)
        return std::nullopt;
    return sin(parameters[0]);
}

// log(x, base)
static std::optional<double> fn_log(const std::vector<double>& parameters) {
    if (parameters.size() != 2)
        return std::nullopt;
    return log(parameters[0]) / log(parameters[1]);
}

int main() {
    std::cout << "please enter an expression: ";
    std::string expression;
    std::getline(std::cin, expression);
    std::cout << "\n";

    auto tokens = expr::tokenize(expression);
    std::cout << tokens << "\n\n";

    const auto tree = expr::parse(std::move(tokens));
    std::cout << tree << "\n";

    const auto symbols = expr::symbol_table{
        {"pi", 3.141592653589793238},
        {"e", 2.718281828459045235}
    };
    const auto functions = expr::function_table{
        {"sin", fn_sin},
        {"log", fn_log}
    };

    const auto result = expr::evaluate(tree, symbols, functions);
    if (result) {
        std::cout << (*result) << std::endl << std::endl;
    } else {
        std::cout << "failed to evaluate" << std::endl << std::endl;
    }

    return 0;
}
