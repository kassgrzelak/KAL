//
// Created by kassie on 12/03/2026.
//
#include "Compiler.h"

#include <stdio.h>

void initCompiler(Compiler* compiler, Bytecode* bytecode, size_t* jumpTable)
{
	compiler->bytecode = bytecode;
	compiler->jumpTable = jumpTable;
	compiler->current = NULL;
}

bool compile(Compiler* compiler, const char* source)
{
	Scanner scanner;
	initScanner(&scanner, source);
	tokenize(&scanner);

	int labelsSeen = 0;

	for (size_t i = 0; i < scanner.tokenArray.count; ++i)
	{
		const Token token = scanner.tokenArray.tokens[i];

		if (token.type != TOKEN_LABEL_DECL)
			continue;
		if (labelsSeen == 256)
			return false; // TODO: Print error.

		compiler->labelNames[labelsSeen] = token.start;
		compiler->labelLengths[labelsSeen++] = token.length;
	}

	for (int i = 0; i < labelsSeen; ++i)
		printf("%.*s\n", compiler->labelLengths[i], compiler->labelNames[i]);

	compiler->current = scanner.tokenArray.tokens;

	freeScanner(&scanner);
	return true;
}
