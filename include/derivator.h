#if !defined(EXPRPARSER_DERIVATOR_HEADER)
#define EXPRPARSER_DERIVATOR_HEADER

#include "node.h"
#include "result.h"

#include <string_view>      // std::string_view

namespace expr {
    using derivator_result = result<node_ptr, error>;

    derivator_result derive(const node_ptr& node, std::string_view variable);
}

#endif
