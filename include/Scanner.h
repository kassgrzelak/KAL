//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_SCANNER_H
#define KAL_SCANNER_H

#include "common.h"

typedef enum
{
	// Instruction mnemonics.
	TOKEN_NOP, TOKEN_LD, TOKEN_OUT, TOKEN_INC, TOKEN_JMP, TOKEN_HLT,

	// Label declaration.
	TOKEN_LABEL_DECL,

	// Operand types.
	TOKEN_CONSTANT, TOKEN_REGISTER, TOKEN_MEMORY, TOKEN_POINTER, TOKEN_LABEL_OPERAND,

	// Special.
	TOKEN_EOF, TOKEN_ERROR
} TokenType;

typedef struct
{
	const char* start;
	TokenType type;
	int length;
	int line;
} Token;

typedef struct
{
	Token* tokens;
	size_t count;
	size_t capacity;
} TokenArray;

typedef struct
{
	const char* start;
	const char* current;
	int line;
	TokenArray tokenArray;
} Scanner;

void initScanner(Scanner* scanner, const char* source);
void freeScanner(const Scanner* scanner);

void tokenize(Scanner* scanner);

#endif //KAL_SCANNER_H
