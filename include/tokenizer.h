#if !defined(EXPRPARSER_TOKENIZER_HEADER)
#define EXPRPARSER_TOKENIZER_HEADER

#include "token.h"
#include "result.h"

namespace expr {
    using tokenizer_result = result<token_list, error>;

    tokenizer_result tokenize(const char *expression);
    tokenizer_result tokenize(const std::string& expression);
}

#endif
