# `exprparser`

This is a study project about parsing mathetmatical expressions. The
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

`exprparser` supports the following "language features": numeric literals,
boolean literals, variable identifiers, function calls, unary numeric operators
(`+`, `-`), binary numeric operators (`+`, `-`, `*`, `/`, `^`), and
sub-expressions (`( ... )`).
