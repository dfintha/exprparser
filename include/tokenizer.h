#if !defined(EXPRPARSER_TOKENIZER_HEADER)
#define EXPRPARSER_TOKENIZER_HEADER

#include "token.h"
#include "result.h"

#include <string_view>      // std::string_view

namespace expr {
    using tokenizer_result = result<token_list, error>;

    tokenizer_result tokenize(const char *expression);
    tokenizer_result tokenize(const std::string& expression);
    tokenizer_result tokenize(std::string_view expression);
}

#endif
