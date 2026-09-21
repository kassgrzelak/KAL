//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_SCANNER_H
#define KAL_SCANNER_H

#include "common.h"
#include "mnemonicTokens.h"

typedef enum
{
#define X(mnemonic) \
	TOKEN_##mnemonic,
	MNEMONIC_TOKENS_X
#undef X

	// Assembler directives.
	TOKEN_DATAFROM, TOKEN_DATATO, TOKEN_MEMALIAS,

	// Label declaration.
	TOKEN_LABEL_DECL,

	// Operand types. All operand types MUST be in a contiguous range.
	TOKEN_CONSTANT, TOKEN_STRING_CHAR, TOKEN_ALIAS_ADDRESS, // All effectively constants.
	TOKEN_REGISTER,
	TOKEN_MEMORY, TOKEN_MEMORY_ALIAS, // All effectively RAM addresses.
	TOKEN_POINTER,
	TOKEN_LABEL_OPERAND,

	// Special.
	TOKEN_EOF, TOKEN_ERROR, TOKEN_SKIP
} TokenType;

bool isStatementStarterType(TokenType type);
bool isOperandType(TokenType type);

typedef enum
{
	BASE_BINARY, BASE_OCTAL, BASE_DECIMAL, BASE_HEXADECIMAL
} NumBase;

bool isAlpha(char c);
bool isDigit(char c, NumBase base);

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
	bool readingString;
	char currentQuoteChar;
} Scanner;

void initScanner(Scanner* scanner, const char* source);
void freeScanner(const Scanner* scanner);

void tokenize(Scanner* scanner);

#endif //KAL_SCANNER_H
