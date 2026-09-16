//
// Created by kassie on 16/09/2026.
//

#ifndef KAL_ASSEMBLER_DIRECTIVES_HPP
#define KAL_ASSEMBLER_DIRECTIVES_HPP
#include "Scanner.h"

typedef struct
{
	TokenType token;
	const char* mnemonic;
} AssemblerDirective;

static const AssemblerDirective assemblerDirectives[] = {
	{TOKEN_DATAFROM, "datafrom"},
	{TOKEN_DATATO, "datato"},
};

static const uint8_t ASSEMBLER_DIRECTIVE_COUNT =  sizeof(assemblerDirectives) / sizeof(AssemblerDirective);

#endif //KAL_ASSEMBLER_DIRECTIVES_HPP
