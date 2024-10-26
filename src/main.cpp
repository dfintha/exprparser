#include "derivator.h"
#include "evaluator.h"
#include "functions.h"
#include "optimizer.h"
#include "parser.h"
#include "tokenizer.h"
#include "version.h"

#include <cstring>          // strdup, std::strlen, std::strncmp
#include <iostream>         // std::cout

#include <readline/readline.h>  // readline, rl_bind_key
#include <readline/history.h>   // add_history, using_history

static void separator(const char *step) {
    static constexpr size_t line_length = 80;
    static const std::string line = std::string(line_length, '-');
    std::cout << "--- " << step << ' '
              << line.substr(0, line_length - std::strlen(step) - 5)
              << "\n\n";
}

template <typename ProcessFn, typename... InputT>
auto process_and_print(
    std::string_view expression,
    const char *action,
    ProcessFn process,
    InputT&&... input
) {
    auto result = process(std::forward<InputT>(input)...);
    if (!result) {
        std::cout << expression << '\n';
        for (size_t i = 0; i < expression.length(); ++i) {
            const bool there = (i + 1) >= result.error().location.begin &&
                               (i + 1) <= result.error().location.end;
            std::cout << (there ? '^' : ' ');
        }
        std::cout << '\n'
                  << "Failed to " << action << ": "
                  << result.error().description
                  << '\n';
    } else {
        std::cout << *result << '\n';
    }

    return std::move(result);
}

void evaluate_and_print(
    const expr::parser_result& root,
    std::string_view tree_kind
) {
    static const auto symbols = expr::symbol_table{
        {"pi", 3.141592653589793238},
        {"e", 2.718281828459045235}
    };

    if (!root)
        return;

    std::cout << "Recreated expression string from "
              << tree_kind << " syntax tree: '"
              << expr::to_expression_string(*root) << "'.\n";

    auto evaluated = expr::evaluate(*root, symbols, expr::functions());
    if (evaluated.has_value()) {
        std::cout << "Evaluation result: " << *evaluated << "\n\n";
    } else {
        std::cout << "Failed to evaluate: "
                  << evaluated.error().description << "\n\n";
    }
}

static int process_expression(std::string_view expression) {
    separator("Tokenization");
    using tokenize_fn = expr::tokenizer_result (*)(std::string_view);
    auto tokens = process_and_print<tokenize_fn>(
        expression,
        "tokenize input",
        expr::tokenize,
        expression
    );
    if (!tokens) {
        std::cout << "Failed to tokenize: "
                  << tokens.error().description << "\n\n";
    }

    separator("Parsing");
    auto parsed = process_and_print(
        expression,
        "parse tokens",
        expr::parse,
        std::move(*tokens)
    );
    evaluate_and_print(parsed, "parsed");

    separator("Optimization");
    auto optimized = process_and_print(
        expression,
        "optimize expression tree",
        expr::optimize,
        *parsed
    );
    evaluate_and_print(optimized, "optimized");

    separator("Derivation");
    auto derived = process_and_print(
        expression,
        "derive expression",
        expr::derive,
        std::move(*parsed)
    );
    evaluate_and_print(derived, "derived");

    return EXIT_SUCCESS;
}

static void initialize_completion() {
    auto completion = [](const char *text, int, int) {
        auto generator = [](const char *text, int state) {
            static bool initialized = false;
            static std::vector<std::string> symbols;
            if (!initialized) {
                const auto& functions = expr::functions();
                symbols.reserve(functions.size());
                for (const auto& [name, _] : functions) {
                    symbols.push_back(name);
                }
                initialized = true;
            }

            static size_t index;
            static size_t length;
            if (state == 0) {
                index = 0;
                length = std::strlen(text);
            }

            const char *name;
            while ((name = symbols[index++].c_str())) {
                if (std::strncmp(name, text, length) == 0) {
                    return strdup(name);
                }
            }

            return static_cast<char *>(nullptr);
        };

        rl_attempted_completion_over = 1;
        rl_completion_suppress_append = 1;
        return rl_completion_matches(text, generator);
    };

    rl_bind_key('\t', rl_complete);
    rl_attempted_completion_function = completion;
}

int main(int argc, char **argv) {
    initialize_completion();
    using_history();

    --argc, ++argv;

    std::cout << expr::program_name << ' ' << expr::program_version
              << " (Built with "
              << expr::program_compiler << ' ' << expr::program_compiler_version
              << " on " << expr::program_platform << ")\n\n";

    if (argc == 0) {
        std::cout << "Available built-in functions:\n";
        std::cout << "    ";
        for (const auto& [_, definition] : expr::functions()) {
            std::cout << definition.signature << ' ';
        }
        std::cout << "\n\n";

        while (true) {
            char *input = readline("exprparser> ");
            if (!input)
                return 0;

            std::cout << std::endl;
            add_history(input);
            process_expression(input);
            std::free(input);
        }

        return 0;
    }

    for (int i = 0; i < argc; ++i) {
        std::cout << '"' << argv[i] << "\"\n\n";
        const int result = process_expression(std::string_view(argv[i]));
        if (result != EXIT_SUCCESS)
            return result;
        std::cout << "\n\n";
    }

    return EXIT_SUCCESS;
}
