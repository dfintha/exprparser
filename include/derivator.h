#if !defined(EXPRPARSER_DERIVATOR_HEADER)
#define EXPRPARSER_DERIVATOR_HEADER

#include "node.h"
#include "result.h"

namespace expr {
    using derivator_result = result<node_ptr, error>;

    derivator_result derive(const node_ptr& node);
}

#endif
