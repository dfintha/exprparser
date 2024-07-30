#include "evaluator.h"
#include "functions.h"
#include "optimizer.h"
#include "parser.h"
#include "tokenizer.h"
#include "version.h"

#include <iostream>

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
                  << " (at location "
                  << result.error().location.begin
                  << "-"
                  << result.error().location.end
                  << ")\n";
    }
    std::cout << *result << '\n';
    return std::move(result);
}

static int process_expression(const std::string& expression) {
    using tokenize_fn = expr::tokenizer_result (*)(const std::string&);
    auto tokens = process_and_print<tokenize_fn>(
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

    std::cout << "recreated expression string from parsed syntax tree: '"
              << expr::to_expression_string(*parsed)
              << "'\n\n";

    auto optimized = process_and_print(
        "optimize expression tree",
        expr::optimize,
        *parsed
    );
    if (!optimized)
        return 3;

    std::cout << "recreated expression string from optimized syntax tree: '"
              << expr::to_expression_string(*optimized)
              << "'\n\n";

    const auto symbols = expr::symbol_table{
        {"pi", 3.141592653589793238},
        {"e", 2.718281828459045235}
    };

    auto result = process_and_print(
        "evaluate expression tree",
        expr::evaluate,
        *optimized,
        symbols,
        expr::functions()
    );
    if (!result)
        return 4;

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    --argc, ++argv;

    std::cout << expr::program_name << ' ' << expr::program_version
              << " (Built with "
              << expr::program_compiler << ' ' << expr::program_compiler_version
              << " on " << expr::program_platform << ")\n\n";

    if (argc == 0) {
        std::cout << "available built-in functions are: ";
        for (const auto& [_, definition] : expr::functions()) {
            std::cout << definition.signature << ' ';
        }
        std::cout << "\n\n";

        std::cout << "please enter an expression: ";
        std::string expression;
        std::getline(std::cin, expression);
        std::cout << '\n';
        return process_expression(expression);
    }

    for (int i = 0; i < argc; ++i) {
        std::cout << '"' << argv[i] << "\"\n\n";
        const int result = process_expression(std::string(argv[i]));
        if (result != EXIT_SUCCESS)
            return result;
        std::cout << "\n\n";
    }

    return EXIT_SUCCESS;
}
