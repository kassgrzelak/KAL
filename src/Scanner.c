//
// Created by kassie on 12/03/2026.
//

#include "Scanner.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "assemblerDirectives.hpp"
#include "instructions.h"
#include "memory.h"

static void initTokenArray(TokenArray* array)
{
	INIT_ARRAY(array, tokens);
}

static void writeTokenArray(TokenArray* array, const Token token)
{
	WRITE_ARRAY(Token, array, tokens, token);
}

static void freeTokenArray(const TokenArray* array)
{
	if (!array)
		return;

	free(array->tokens);
}

bool isStatementStarter(const TokenType type)
{
	return type <= TOKEN_LABEL_DECL;
}

bool isOperand(const TokenType type)
{
	return type >= TOKEN_CONSTANT && type <= TOKEN_LABEL_OPERAND;
}

void initScanner(Scanner* scanner, const char* source)
{
	scanner->start = source;
	scanner->current = source;
	scanner->line = 1;
	initTokenArray(&scanner->tokenArray);
	scanner->readingString = false;
}

void freeScanner(const Scanner* scanner)
{
	freeTokenArray(&scanner->tokenArray);
}

#ifdef DEBUG_PRINT
static void printToken(const TokenType type, const char* start, const int length, const int line)
{
	printf("%04d |", line);

#define TYPE_CASE(type) \
	case type: \
		printf(" %-20s |", #type); \
		break

	switch (type)
	{
#define X(mnemonic) \
	TYPE_CASE(TOKEN_##mnemonic);
		MNEMONIC_TOKENS_X
#undef X

		TYPE_CASE(TOKEN_DATAFROM);
		TYPE_CASE(TOKEN_DATATO);
		TYPE_CASE(TOKEN_MEMALIAS);

		TYPE_CASE(TOKEN_LABEL_DECL);

		TYPE_CASE(TOKEN_CONSTANT);
		TYPE_CASE(TOKEN_STRINGCHAR);
		TYPE_CASE(TOKEN_REGISTER);
		TYPE_CASE(TOKEN_MEMORY);
		TYPE_CASE(TOKEN_POINTER);
		TYPE_CASE(TOKEN_LABEL_OPERAND);

		TYPE_CASE(TOKEN_EOF);
		TYPE_CASE(TOKEN_ERROR);

	default:
		printf("           UNKNOWN |");
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

#ifdef DEBUG_PRINT
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

#ifdef DEBUG_PRINT
	printToken(TOKEN_ERROR, message, token.length, token.line);
#endif

	// Advance past error.
	++scanner->current;

	return token;
}

static Token errorTokenNoAdvance(const Scanner* scanner, const char* message)
{
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner->line;

#ifdef DEBUG_PRINT
	printToken(TOKEN_ERROR, message, token.length, token.line);
#endif

	return token;
}

bool isAlpha(const char c)
{
	return (c >= 'a' && c <= 'z') ||
		   (c >= 'A' && c <= 'Z') ||
		   	c == '_';
}

bool isDigit(const char c, const NumBase base)
{
	switch (base)
	{
	case BASE_BINARY:
		return c == '0' || c == '1';
	case BASE_OCTAL:
		return c >= '0' && c <= '7';
	case BASE_DECIMAL:
		return c >= '0' && c <= '9';
	case BASE_HEXADECIMAL:
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

static void skip(Scanner* scanner)
{
	advance(scanner);
	scanner->start = scanner->current;
}

static char peek(const Scanner* scanner)
{
	return *scanner->current;
}

static bool skipWhitespace(Scanner* scanner)
{
	bool whitespaceSeen = false;

	for (;;)
	{
		switch (peek(scanner))
		{
		case ' ':
		case '\r':
		case'\t':
			whitespaceSeen = true;
			advance(scanner);
			break;
		case ';':
			{
				whitespaceSeen = true;
				advance(scanner);

				while (peek(scanner) != '\n' && peek(scanner) != ';' && !atEnd(scanner))
					advance(scanner);

				// Consume inline comment.
				if (peek(scanner) == ';')
					advance(scanner);

				break;
			}
		case '\n':
			++scanner->line;
			whitespaceSeen = true;
			advance(scanner);
			break;

		default:
			return whitespaceSeen;
		}
	}
}

static Token mnemonic(Scanner* scanner)
{
	const size_t length = scanner->current - scanner->start;

	for (int i = 0; i < INSTR_COUNT; ++i)
	{
		const char* mnemonic = instrTable[i].mnemonic;

		if (length != strlen(mnemonic))
			skip:
			continue;

		for (size_t j = 0; j < length; ++j)
			if (tolower(scanner->start[j]) != mnemonic[j])
				goto skip;

		return makeToken(scanner, instrTable[i].token);
	}

	return errorToken(scanner, "Unknown instruction mnemonic.");
}

static Token assemblerDirective(Scanner* scanner)
{
	const size_t length = scanner->current - scanner->start;

	for (int i = 0; i < ASSEMBLER_DIRECTIVE_COUNT; ++i)
	{
		const char* mnemonic = assemblerDirectives[i].mnemonic;

		if (length != strlen(mnemonic))
			skip:
			continue;

		for (size_t j = 0; j < length; ++j)
			if (tolower(scanner->start[j]) != mnemonic[j])
				goto skip;

		return makeToken(scanner, assemblerDirectives[i].token);
	}

	return errorToken(scanner, "Unknown assembler directive.");
}

static Token identifier(Scanner* scanner, const bool isLabelOperand, const bool isNamedMemDecl,
	const bool isNamedMemAddr, const bool isAssemblerDirective)
{
	while (isAlpha(peek(scanner)) || isDigit(peek(scanner), BASE_DECIMAL))
		advance(scanner);

	if (isLabelOperand)
		return makeToken(scanner, TOKEN_LABEL_OPERAND);
	if (isNamedMemDecl)
		return makeToken(scanner, TOKEN_MEMORY);
	if (isNamedMemAddr)
		return makeToken(scanner, TOKEN_CONSTANT);
	if (isAssemblerDirective)
		return assemblerDirective(scanner);

	if (peek(scanner) == ':')
	{
		advance(scanner);
		return makeToken(scanner, TOKEN_LABEL_DECL);
	}

	return mnemonic(scanner);
}

static Token number(Scanner* scanner, const TokenType type)
{
	if (type == TOKEN_REGISTER && peek(scanner) >= 'a' && peek(scanner) <= 'h')
	{
		// Convert letters a-h to numbers 0-7.
		const int registerNum = peek(scanner) - 'a';
		advance(scanner);
		const char* digit;

		switch (registerNum)
		{
		case 0: digit = "0"; break;
		case 1: digit = "1"; break;
		case 2: digit = "2"; break;
		case 3: digit = "3"; break;
		case 4: digit = "4"; break;
		case 5: digit = "5"; break;
		case 6: digit = "6"; break;
		case 7: digit = "7"; break;

		default: digit = "!"; break; // Unreachable as we check for a <= letter <= h.
		}

#ifdef DEBUG_PRINT
		printToken(type, digit, 1, scanner->line);
#endif
		return (Token){digit, type, 1, scanner->line};
	}

	if (!isDigit(peek(scanner), BASE_DECIMAL))
		return errorToken(scanner, "Expected number or base specifier after operator.");

	NumBase base = BASE_DECIMAL;

	if (peek(scanner) == '0')
	{
		advance(scanner);

		if (isDigit(peek(scanner), BASE_DECIMAL))
			base = BASE_OCTAL;
		else
		{
			switch (tolower(peek(scanner)))
			{
			case 'b':
				base = BASE_BINARY;
				advance(scanner);
				break;
			case 'x':
				base = BASE_HEXADECIMAL;
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

static Token stringChar(Scanner* scanner)
{
	advance(scanner);
	return makeToken(scanner, TOKEN_STRINGCHAR);
}

static Token scanToken(Scanner* scanner)
{
	if (atEnd(scanner))
		return makeToken(scanner, TOKEN_EOF);

	const char c = peek(scanner);

	if (c != '"' && scanner->readingString)
	{
		return stringChar(scanner);
	}

	if (c == '"' && scanner->readingString)
	{
		skip(scanner);
		scanner->readingString = false;
		const Token skipToken = {NULL, TOKEN_SKIP, 0, 0};
		return skipToken;
	}

	if (c == '"')
	{
		skip(scanner);
		scanner->readingString = true; // Must be false at this point in the code due to above ifs.
		return stringChar(scanner);
	}

	if (isAlpha(c))
		return identifier(scanner, false, false, false, false);
	if (isDigit(c, BASE_DECIMAL))
		return number(scanner, TOKEN_CONSTANT);
	if (c == '%')
	{
		skip(scanner);
		return number(scanner, TOKEN_REGISTER);
	}
	if (c == '$')
	{
		skip(scanner);

		if (isDigit(peek(scanner), BASE_DECIMAL))
			return number(scanner, TOKEN_MEMORY);
		return identifier(scanner, false, true, false, false);
	}
	if (c == '*')
	{
		skip(scanner);
		return number(scanner, TOKEN_POINTER);
	}
	if (c == '.')
	{
		skip(scanner);

		if (!isAlpha(peek(scanner)))
			return errorToken(scanner, "Expected label name after label operator.");

		return identifier(scanner, true, false, false, false);
	}
	if (c == '#')
	{
		skip(scanner);

		if (!isAlpha(peek(scanner)))
			return errorToken(scanner, "Expected assembler directive after assembler directive operator.");

		return identifier(scanner, false, false, false, true);
	}
	if (c == '&')
	{
		skip(scanner);

		if (!isAlpha(peek(scanner)))
			return errorToken(scanner, "Expected named RAM identifier after named RAM address operator.");

		return identifier(scanner, false, false, true, false);
	}

	return errorToken(scanner, "Unexpected character.");
}

void tokenize(Scanner* scanner)
{
#ifdef DEBUG_PRINT
	printf("\n=== TOKENS ===\n");
	printf("Line | Token type           | Lexeme\n");
	printf("-------------------------------------\n");
#endif

	skipWhitespace(scanner);
	scanner->start = scanner->current;

	for (;;)
	{
		const Token token = scanToken(scanner);
		
		if (token.type == TOKEN_EOF)
		{
			if (scanner->readingString)
				writeTokenArray(&scanner->tokenArray, errorTokenNoAdvance(scanner, "End of file reached before"
					" closing double quote of string."));
			
			writeTokenArray(&scanner->tokenArray, token);
			break;
		}
		
		writeTokenArray(&scanner->tokenArray, token);

		if (scanner->readingString)
		{
			scanner->start = scanner->current;
			continue;
		}

		if (!skipWhitespace(scanner))
			writeTokenArray(&scanner->tokenArray, errorTokenNoAdvance(scanner, "Expected whitespace before "
				"next token. This was likely caused by a malformed integer literal."));

		scanner->start = scanner->current;
	}
}
