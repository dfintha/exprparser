#if !defined(EXPRPARSER_TOKENIZER_HEADER)
#define EXPRPARSER_TOKENIZER_HEADER

#include "token.h"

namespace expr {
    token_list tokenize(const char *expression);
    token_list tokenize(const std::string& expression);
}

#endif
