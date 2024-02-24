#if !defined(EXPRPARSER_OPTIMIZER_HEADER)
#define EXPRPARSER_OPTIMIZER_HEADER

#include "node.h"

namespace expr {
    node_ptr optimize(const node_ptr& root);
}

#endif
