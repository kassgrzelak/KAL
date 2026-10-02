//
// Created by kassie on 23/09/2026.
//

#include "PreprocScanner.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "preprocDirectives.h"
#include "Scanner.h"

static void initPreprocTokenArray(PreprocTokenArray* array)
{
	INIT_ARRAY(array, tokens);
}

static void writePreprocTokenArray(PreprocTokenArray* array, const PreprocToken token)
{
	WRITE_ARRAY(PreprocToken, array, tokens, token);
}

static void freePreprocTokenArray(const PreprocTokenArray* array)
{
	if (!array)
		return;

	free(array->tokens);
}

void initPreprocScanner(PreprocScanner* scanner, const char* source)
{
	scanner->start = source;
	scanner->current = source;
	scanner->line = 1;
	initPreprocTokenArray(&scanner->tokenArray);
}

void freePreprocScanner(const PreprocScanner* scanner)
{
	freePreprocTokenArray(&scanner->tokenArray);
}

static bool atEnd(const PreprocScanner* scanner)
{
	return *scanner->current == '\0';
}

static char advance(PreprocScanner* scanner)
{
	return *scanner->current++;
}

static void skip(PreprocScanner* scanner)
{
	advance(scanner);
	scanner->start = scanner->current;
}

static char peek(const PreprocScanner* scanner)
{
	return *scanner->current;
}

static bool skipWhitespace(PreprocScanner* scanner)
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

#ifdef DEBUG_PRINT
static void printToken(const PreprocTokenType type, const char* start, const int length, const int line)
{
	printf("%04d |", line);

#define TYPE_CASE(type) \
	case type: \
	printf(" %-25s |", #type); \
	break

	switch (type)
	{
		TYPE_CASE(PREPROC_TOKEN_MACRO);
		TYPE_CASE(PREPROC_TOKEN_UNMACRO);

		TYPE_CASE(PREPROC_TOKEN_IDENTIFIER);
		TYPE_CASE(PREPROC_TOKEN_REPLACEMENT);

		TYPE_CASE(PREPROC_TOKEN_TEXT);

		TYPE_CASE(PREPROC_TOKEN_EOF);
		TYPE_CASE(PREPROC_TOKEN_ERROR);

	default:
		printf("           UNKNOWN |");
		break;
	}

	printf(" '%.*s'\n", length, start);
#undef TYPE_CASE
}
#endif

// Make a token out of the characters the scanner has consumed.
static PreprocToken makeToken(const PreprocScanner* scanner, const PreprocTokenType type)
{
	PreprocToken token;
	token.type = type;
	token.start = scanner->start;
	token.length = (int)(scanner->current - scanner->start);
	token.line = scanner->line;

#ifdef DEBUG_PRINT
	printToken(type, token.start, token.length, token.line);
#endif

	return token;
}

static PreprocToken errorToken(PreprocScanner* scanner, const char* message)
{
	PreprocToken token;
	token.type = PREPROC_TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner->line;

#ifdef DEBUG_PRINT
	printToken(PREPROC_TOKEN_ERROR, message, token.length, token.line);
#endif

	// Advance past error.
	++scanner->current;

	return token;
}

static void macroDirective(PreprocScanner* scanner)
{
	skipWhitespace(scanner);
	scanner->start = scanner->current;
	
	if (!isAlpha(peek(scanner)))
	{
		writePreprocTokenArray(&scanner->tokenArray, errorToken(scanner, "Expected identifier after #macro"
			" directive."));
		return;
	}

	while (isAlpha(peek(scanner)) || isDigit(peek(scanner), BASE_DECIMAL))
		advance(scanner);

	writePreprocTokenArray(&scanner->tokenArray, makeToken(scanner, PREPROC_TOKEN_IDENTIFIER));

	skipWhitespace(scanner);

	if (peek(scanner) != '`')
	{
		writePreprocTokenArray(&scanner->tokenArray, errorToken(scanner, "Expected macro definition after "
			"#macro directive."));
		return;
	}

	skip(scanner);
	
	while (peek(scanner) != '`' && !atEnd(scanner))
		advance(scanner);
	
	if (atEnd(scanner))
	{
		writePreprocTokenArray(&scanner->tokenArray, errorToken(scanner, "End of file reached before closing"
			" quote of macro definition."));
		return;
	}
	
	writePreprocTokenArray(&scanner->tokenArray, makeToken(scanner, PREPROC_TOKEN_REPLACEMENT));
	skip(scanner); // Skip closing quote.
}

static void unmacroDirective(PreprocScanner* scanner)
{
	skipWhitespace(scanner);
	scanner->start = scanner->current;
	
	if (!isAlpha(peek(scanner)))
	{
		writePreprocTokenArray(&scanner->tokenArray, errorToken(scanner, "Expected identifier after #unmacro"
			" directive."));
		return;
	}

	while (isAlpha(peek(scanner)) || isDigit(peek(scanner), BASE_DECIMAL))
		advance(scanner);

	writePreprocTokenArray(&scanner->tokenArray, makeToken(scanner, PREPROC_TOKEN_IDENTIFIER));
	scanner->start = scanner->current;
}

void preprocTokenize(PreprocScanner* scanner)
{
#ifdef DEBUG_PRINT
	printf("\n=== PREPROCESSOR TOKENS ===\n");
	printf("Line | Token type           | Lexeme\n");
	printf("-------------------------------------\n");
#endif

	for (;;)
	{
		if (atEnd(scanner))
		{
			endOfText:
			writePreprocTokenArray(&scanner->tokenArray, makeToken(scanner, PREPROC_TOKEN_EOF));
			break;
		}
		
		while (peek(scanner) != '#' && !atEnd(scanner))
			advance(scanner);
		
		writePreprocTokenArray(&scanner->tokenArray, makeToken(scanner, PREPROC_TOKEN_TEXT));
		scanner->start = scanner->current;

		if (atEnd(scanner))
			goto endOfText;

		advance(scanner); // Consume '#' char and reset consumed characters.

		while (isAlpha(peek(scanner)))
			advance(scanner);

		const size_t length = scanner->current - scanner->start - 1;
		PreprocTokenType tokenType = PREPROC_TOKEN_ERROR;

		for (int i = 0; i < PREPROC_DIRECTIVE_COUNT; ++i)
		{
			const char* mnemonic = preprocDirectives[i].mnemonic;

			if (length != strlen(mnemonic))
				skip:
				continue;

			for (size_t j = 0; j < length; ++j)
				if (tolower(scanner->start[j + 1]) != mnemonic[j])
					goto skip;

			tokenType = preprocDirectives[i].token;
			break;
		}

		if (tokenType == PREPROC_TOKEN_ERROR)
			continue; // Go back to consuming as text. The non-preprocessor directive stays in the consumed characters.

		writePreprocTokenArray(&scanner->tokenArray, makeToken(scanner, tokenType));

		switch (tokenType)
		{
		case PREPROC_TOKEN_MACRO: macroDirective(scanner); break;
		case PREPROC_TOKEN_UNMACRO: unmacroDirective(scanner); break;

		default: break;
		}
	}
}
