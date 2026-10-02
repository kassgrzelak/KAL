//
// Created by kassie on 25/09/2026.
//

#ifndef KAL_PREPROCESSOR_DIRECTIVES_HPP
#define KAL_PREPROCESSOR_DIRECTIVES_HPP
#include <stdint.h>

#include "PreprocScanner.h"

typedef struct
{
	PreprocTokenType token;
	const char* mnemonic;
} PreprocDirective;

static const PreprocDirective preprocDirectives[] = {
	{PREPROC_TOKEN_MACRO, "macro"},
	{PREPROC_TOKEN_UNMACRO, "unmacro"},
};

static const uint8_t PREPROC_DIRECTIVE_COUNT =  sizeof(preprocDirectives) / sizeof(PreprocDirective);

#endif //KAL_PREPROCESSOR_DIRECTIVES_HPP
