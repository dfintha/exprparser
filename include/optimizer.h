#if !defined(EXPRPARSER_OPTIMIZER_HEADER)
#define EXPRPARSER_OPTIMIZER_HEADER

#include "node.h"
#include "result.h"

namespace expr {
    using optimizer_result = result<node_ptr, error>;

    optimizer_result optimize(const node_ptr& root);
}

#endif
