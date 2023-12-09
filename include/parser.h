#if !defined(EXPRPARSER_PARSER_HEADER)
#define EXPRPARSER_PARSER_HEADER

#include "node.h"
#include "token.h"

namespace expr {
    node_ptr parse(token_list&& tokens);
}

#endif
