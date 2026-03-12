//
// Created by kassie on 12/03/2026.
//

#include "Scanner.h"

#include <stdio.h>
#include <string.h>

#include "memory.h"

static void initTokenArray(TokenArray* array)
{
	INIT_ARRAY(array, tokens);
}

static void writeTokenArray(TokenArray* array, const Token token)
{
	WRITE_ARRAY(Token, array, tokens, token);
}

void initScanner(Scanner* scanner, const char* source)
{
	scanner->start = source;
	scanner->current = source;
	scanner->line = 1;
	initTokenArray(&scanner->tokenArray);
}

void freeScanner(const Scanner* scanner)
{
	free(scanner->tokenArray.tokens);
}

#ifdef DEBUG_PRINT_TOKENS
static void printToken(const TokenType type, const char* start, const int length, const int line)
{
	printf("%04d |", line);

#define TYPE_CASE(type) \
	case type: \
		printf(" %-20s |", #type); \
		break

	switch (type)
	{
	TYPE_CASE(TOKEN_MNEMONIC);

	TYPE_CASE(TOKEN_LABEL_DECL);

	TYPE_CASE(TOKEN_CONSTANT);
	TYPE_CASE(TOKEN_REGISTER);
	TYPE_CASE(TOKEN_MEMORY);
	TYPE_CASE(TOKEN_POINTER);
	TYPE_CASE(TOKEN_LABEL_OPERAND);

	TYPE_CASE(TOKEN_EOF);
	TYPE_CASE(TOKEN_ERROR);

	default:
		printf(" UNKNOWN");
		break;
	}

	printf(" '%.*s'\n", length, start);
#undef TYPE_CASE
}
#endif

static Token makeToken(const Scanner* scanner, const TokenType type)
{
	Token token;
	token.type = type;
	token.start = scanner->start;
	token.length = (int)(scanner->current - scanner->start);
	if (type == TOKEN_LABEL_DECL)
		token.length -= 1;
	token.line = scanner->line;

#ifdef DEBUG_PRINT_TOKENS
	printToken(type, token.start, token.length, token.line);
#endif

	return token;
}

static Token errorToken(Scanner* scanner, const char* message)
{
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner->line;

#ifdef DEBUG_PRINT_TOKENS
	printToken(TOKEN_ERROR, message, token.length, token.line);
#endif

	// Advance past error.
	++scanner->current;

	return token;
}

static bool isAlpha(const char c)
{
	return (c >= 'a' && c <= 'z') ||
		   (c >= 'A' && c <= 'Z');
}

typedef enum
{
	BINARY, OCTAL, DECIMAL, HEXADECIMAL
} NumBase;

static bool isDigit(const char c, const NumBase base)
{
	switch (base)
	{
	case BINARY:
		return c == '0' || c == '1';
	case OCTAL:
		return c >= '0' && c <= '7';
	case DECIMAL:
		return c >= '0' && c <= '9';
	case HEXADECIMAL:
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	return false;
}

static bool atEnd(const Scanner* scanner)
{
	return *scanner->current == '\0';
}

static char advance(Scanner* scanner)
{
	return *scanner->current++;
}

static char peek(const Scanner* scanner)
{
	return *scanner->current;
}

static void skipWhitespace(Scanner* scanner)
{
	for (;;)
	{
		switch (peek(scanner))
		{
		case ' ':
		case '\r':
		case'\t':
			advance(scanner);
			break;
		case ';':
			while (peek(scanner) != '\n' && !atEnd(scanner))
				advance(scanner);
			break;
		case '\n':
			++scanner->line;
			advance(scanner);
			break;

		default:
			return;
		}
	}
}

static Token identifier(Scanner* scanner, const bool labelOperand)
{
	while (isAlpha(peek(scanner)) || isDigit(peek(scanner), DECIMAL))
		advance(scanner);

	if (labelOperand)
		return makeToken(scanner, TOKEN_LABEL_OPERAND);

	TokenType type = TOKEN_MNEMONIC;

	if (peek(scanner) == ':')
	{
		advance(scanner);
		type = TOKEN_LABEL_DECL;
	}

	return makeToken(scanner, type);
}

static Token number(Scanner* scanner, const TokenType type)
{
	if (!isDigit(peek(scanner), DECIMAL))
		return errorToken(scanner, "Expected number or base specifier after operator.");

	NumBase base = DECIMAL;

	if (peek(scanner) == '0')
	{
		advance(scanner);

		if (isDigit(peek(scanner), OCTAL))
		{
			advance(scanner);
			base = OCTAL;
		}
		else
		{
			switch (peek(scanner))
			{
			case 'b':
				base = BINARY;
				advance(scanner);
				break;
			case 'x':
				base = HEXADECIMAL;
				advance(scanner);
				break;

			default:
				break;
			}
		}
	}

	while (isDigit(peek(scanner), base))
		advance(scanner);

	return makeToken(scanner, type);
}

static Token scanToken(Scanner* scanner)
{
	skipWhitespace(scanner);
	scanner->start = scanner->current;

	if (atEnd(scanner))
		return makeToken(scanner, TOKEN_EOF);

	const char c = peek(scanner);

	if (isAlpha(c))
		return identifier(scanner, false);
	if (isDigit(c, DECIMAL))
		return number(scanner, TOKEN_CONSTANT);
	if (c == '%')
	{
		advance(scanner);
		scanner->start = scanner->current;
		return number(scanner, TOKEN_REGISTER);
	}
	if (c == '$')
	{
		advance(scanner);
		scanner->start = scanner->current;
		return number(scanner, TOKEN_MEMORY);
	}
	if (c == '*')
	{
		advance(scanner);
		scanner->start = scanner->current;
		return number(scanner, TOKEN_POINTER);
	}
	if (c == '.')
	{
		advance(scanner);
		scanner->start = scanner->current;

		if (!isAlpha(peek(scanner)))
			return errorToken(scanner, "Expected label name after label operator.");

		return identifier(scanner, true);
	}

	return errorToken(scanner, "Unexpected character.");
}

void tokenize(Scanner* scanner)
{
#ifdef DEBUG_PRINT_TOKENS
	printf("Line | Token type           | Lexeme\n");
	printf("------------------------------------\n");
#endif

	for (;;)
	{
		const Token token = scanToken(scanner);
		writeTokenArray(&scanner->tokenArray, token);
		if (token.type == TOKEN_EOF)
			break;
	}
}
