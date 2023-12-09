#include "evaluator.h"
#include "parser.h"
#include "tokenizer.h"

#include <iostream>

int main() {
    std::cout << "please enter an expression: ";
    std::string expression;
    std::getline(std::cin, expression);
    std::cout << "\n";

    auto tokens = expr::tokenize(expression);
    std::cout << tokens << "\n\n";

    const auto tree = expr::parse(std::move(tokens));
    std::cout << tree << "\n";

    const auto symbols = expr::symbol_table{{"pi", 3.14159265358}};
    const auto result = expr::evaluate(tree, symbols);
    if (result) {
        std::cout << (*result) << std::endl << std::endl;
    } else {
        std::cout << "failed to evaluate" << std::endl << std::endl;
    }

    return 0;
}
