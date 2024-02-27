#if !defined(EXPRPARSER_PARSER_HEADER)
#define EXPRPARSER_PARSER_HEADER

#include "node.h"
#include "result.h"
#include "token.h"

namespace expr {
    using parser_result = result<node_ptr, error>;

    parser_result parse(token_list&& tokens);
}

#endif
