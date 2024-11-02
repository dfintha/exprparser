# `exprparser`

This is a study project about parsing mathematical expressions. The
implementation consists of a state-machine-based
[tokenizer](https://en.wikipedia.org/wiki/Lexical_analysis), a
[recursive descent](https://en.wikipedia.org/wiki/Recursive_descent_parser)
parser, a simple
[peephole optimizer](https://en.wikipedia.org/wiki/Peephole_optimization), and a
recursive evaluator.

The demo application can be built with
[barge](https://github.com/dfintha/barge).
It consists of parsing an expression entered by the user, and printing the
token stream, the parsed expression tree, the optimized expression tree, and
the result of the expression's evaluation to the standard output.

The software uses features of the C++17 standard, so when building in with
another build system, such standard such be given.

`exprparser` supports the following "language features": numeric literals,
variable identifiers, variable assignments, function calls, unary numeric
operators (`+`, `-`), binary numeric operators (`+`, `-`, `*`, `/`, `^`), and
sub-expressions (`( ... )`).
