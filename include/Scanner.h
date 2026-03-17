//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_SCANNER_H
#define KAL_SCANNER_H

#include "common.h"
#include "mnemonicTokens.inc"

typedef enum
{
#define X(mnemonic) \
	TOKEN_##mnemonic,
	MNEMONIC_TOKENS_X
#undef X

	// Label declaration.
	TOKEN_LABEL_DECL,

	// Operand types. All operand types MUST be in a contiguous range.
	TOKEN_CONSTANT, TOKEN_REGISTER, TOKEN_MEMORY, TOKEN_POINTER, TOKEN_LABEL_OPERAND,

	// Special.
	TOKEN_EOF, TOKEN_ERROR
} TokenType;

bool isStatementStarter(TokenType type);
bool isOperand(TokenType type);

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
