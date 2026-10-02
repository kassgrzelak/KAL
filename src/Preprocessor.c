//
// Created by kassie on 26/09/2026.
//

#include "Preprocessor.h"

#include <stdio.h>
#include <string.h>

#include "Compiler.h"
#include "PreprocScanner.h"

typedef struct
{
	char* processedText;
	size_t processedTextLength;

	int macroDirectivesSeen;

	const char* macroIdentifiers[256];
	int macroIdentifierLengths[256];
	const char* macroReplacements[256];
	int macroReplacementLengths[256];

	bool hadError;
	bool madeReplacements;
	bool finalRound;

	const PreprocToken* current;
} Preprocessor;

static void initPreprocessor(Preprocessor* preprocessor, const PreprocScanner* scanner, const bool finalRound)
{
	preprocessor->processedText = NULL;
	preprocessor->processedTextLength = 0;
	preprocessor->macroDirectivesSeen = 0;
	for (int i = 0; i < 256; ++ i)
		preprocessor->macroIdentifiers[i] = NULL;
	preprocessor->hadError = false;
	preprocessor->madeReplacements = false;
	preprocessor->finalRound = finalRound;
	preprocessor->current = scanner->tokenArray.tokens;
}

static void errorAt(Preprocessor* preprocessor, const PreprocToken* token, const char* message)
{
	preprocessor->hadError = true;

	printf("[line %d] Preprocessor Error", token->line);

	if (token->type == PREPROC_TOKEN_EOF)
		printf(" at end");
	else if (token->type == PREPROC_TOKEN_ERROR)
	{ /* Nothing. */ }
	else
		printf(" at '%.*s'", token->length, token->start);

	printf(": %s\n", message);
}

static const PreprocToken* peek(const Preprocessor* preprocessor)
{
	return preprocessor->current;
}

static void advance(Preprocessor* preprocessor)
{
	for (;;)
	{
		++preprocessor->current;
		if (preprocessor->current->type != PREPROC_TOKEN_ERROR)
			break;

		errorAt(preprocessor, preprocessor->current, preprocessor->current->start);
	}
}

static bool appendChars(Preprocessor* preprocessor, const char* start, const char* end)
{
	const size_t length = end - start;
	char* newText = realloc(preprocessor->processedText, preprocessor->processedTextLength + length);

	if (newText == NULL)
		return false;

	preprocessor->processedText = newText;

	memcpy(preprocessor->processedText + preprocessor->processedTextLength, start, length);
	preprocessor->processedTextLength += length;

	return true;
}

static bool appendString(Preprocessor* preprocessor, const char* string)
{
	return appendChars(preprocessor, string, string + strlen(string));
}

static void macroDirective(Preprocessor* preprocessor)
{
	const PreprocToken* directiveToken = peek(preprocessor);
	advance(preprocessor);

	if (preprocessor->macroDirectivesSeen >= 256)
		return errorAt(preprocessor, directiveToken, "Exceeded the maximum of 256 macros in one program.");

	const PreprocToken* identifierToken = peek(preprocessor);

	if (identifierToken->type != PREPROC_TOKEN_IDENTIFIER)
		return;

	const char* macroIdentifier = identifierToken->start;
	const int identifierLength = identifierToken->length;

	advance(preprocessor);
	const PreprocToken* replacementToken = peek(preprocessor);

	if (replacementToken->type != PREPROC_TOKEN_REPLACEMENT)
		return;

	const char* replacement = replacementToken->start;
	const int replacementLength = replacementToken->length;

	const int index = preprocessor->macroDirectivesSeen;

	preprocessor->macroIdentifiers[index] = macroIdentifier;
	preprocessor->macroIdentifierLengths[index] = identifierLength;
	preprocessor->macroReplacements[index] = replacement;
	preprocessor->macroReplacementLengths[index] = replacementLength;

	++preprocessor->macroDirectivesSeen;
	advance(preprocessor);

	if (!preprocessor->finalRound)
	{
		appendString(preprocessor, "#macro ");
		appendChars(preprocessor, macroIdentifier, macroIdentifier + identifierLength);
		appendString(preprocessor, " `");
		appendChars(preprocessor, replacement, replacement + replacementLength);
		appendString(preprocessor, "` ");
	}
}

static void unmacroDirective(Preprocessor* preprocessor)
{
	advance(preprocessor);

	const PreprocToken* identifierToken = peek(preprocessor);

	if (identifierToken->type != PREPROC_TOKEN_IDENTIFIER)
		return;

	const int macroIndex = findName(preprocessor->macroIdentifiers, preprocessor->macroIdentifierLengths,
		identifierToken->start, identifierToken->length);

	if (macroIndex == -1)
		return errorAt(preprocessor, identifierToken, "Tried to undefine non-existent macro.");

	if (!preprocessor->finalRound)
	{
		const char* identifier = preprocessor->macroIdentifiers[macroIndex];
		const int identifierLength = preprocessor->macroIdentifierLengths[macroIndex];

		appendString(preprocessor, "#unmacro ");
		appendChars(preprocessor, identifier, identifier + identifierLength);
		appendString(preprocessor, " ");
	}

	preprocessor->macroIdentifiers[macroIndex] = NULL;
	advance(preprocessor);
}

static void processText(Preprocessor* preprocessor)
{
	const PreprocToken* textToken = peek(preprocessor);

	const char* textStart = textToken->start;
	const char* textEnd = textToken->start;
	const char* identifierEnd = textToken->start;

	while (textEnd - textToken->start < textToken->length)
	{
		if (isAlpha(*textEnd))
		{
			while ((isAlpha(*identifierEnd) || isDigit(*identifierEnd, BASE_DECIMAL))
				&& identifierEnd - textToken->start <= textToken->length)
				++identifierEnd;

			const int macroIndex = findName(preprocessor->macroIdentifiers, preprocessor->macroIdentifierLengths,
				textEnd, (int)(identifierEnd - textEnd));

			if (macroIndex == -1)
				textEnd = identifierEnd;
			else
			{
				preprocessor->madeReplacements = true;

				appendChars(preprocessor, textStart, textEnd);

				const char* replacementStart = preprocessor->macroReplacements[macroIndex];
				const char* replacementEnd = replacementStart + preprocessor->macroReplacementLengths[macroIndex];

				appendChars(preprocessor, replacementStart, replacementEnd);

				textStart = identifierEnd;
				textEnd = identifierEnd;
			}
		}

		++textEnd;
		++identifierEnd;
	}

	if (textEnd - textStart > 0)
		appendChars(preprocessor, textStart, textEnd);

	advance(preprocessor);
}

#ifdef DEBUG_PRINT
static int preprocessingRounds = 0;
#endif

static bool doPreprocessRound(const char* source, char** processedText, bool* madeReplacements, const bool finalRound)
{
#ifdef DEBUG_PRINT
	printf("\nDoing ");

	if (finalRound)
		printf("final ");

	printf("preprocessing round");

	if (finalRound)
		printf(".\n");
	else
		printf(" %d.\n", ++preprocessingRounds);
#endif

	PreprocScanner scanner;
	initPreprocScanner(&scanner, source);
	preprocTokenize(&scanner);

	Preprocessor preprocessor;
	initPreprocessor(&preprocessor, &scanner, finalRound);

	while (peek(&preprocessor)->type != PREPROC_TOKEN_EOF)
	{
		switch (peek(&preprocessor)->type)
		{
		case PREPROC_TOKEN_MACRO:
			macroDirective(&preprocessor); break;
		case PREPROC_TOKEN_UNMACRO:
			unmacroDirective(&preprocessor); break;
		case PREPROC_TOKEN_TEXT:
			processText(&preprocessor); break;
		case PREPROC_TOKEN_ERROR:
			errorAt(&preprocessor, preprocessor.current, preprocessor.current->start);
			advance(&preprocessor);
			break;

		default:
			errorAt(&preprocessor, preprocessor.current, "Unexpected token.");
			advance(&preprocessor);
			break;
		}
	}

	const char* nullEnd = "\0";
	appendChars(&preprocessor, nullEnd, nullEnd + 1);

	*processedText = preprocessor.processedText;
	*madeReplacements = preprocessor.madeReplacements;
	return !preprocessor.hadError;
}

// Preprocess source text and return whether it was successful.
bool preprocess(char* source, char** processedText)
{
	bool madeReplacements = true;
	bool success = true;
	int roundCount = 0;

	while (madeReplacements && roundCount <= 64)
	{
		madeReplacements = false;
		++roundCount;
		char* roundProcessedText;

		const bool roundSuccess = doPreprocessRound(source, &roundProcessedText, &madeReplacements, false);

		if (roundCount != 1)
			free(source);
		source = roundProcessedText;
		if (!roundSuccess)
		{
			success = false;
			break;
		}
	}

	if (roundCount > 64)
	{
		printf("\nExceeded maximum number of preprocessing rounds.\n");
		return false;
	}

	char* finalProcessedText;
	success = doPreprocessRound(source, &finalProcessedText, &madeReplacements, true);

	free(source);
	source = finalProcessedText;

#ifdef DEBUG_PRINT
	printf("\nFinal processed text:\n\"%s\"", finalProcessedText);
#endif

	*processedText = source;
	return success;
}
