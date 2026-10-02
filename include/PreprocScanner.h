//
// Created by kassie on 23/09/2026.
//

#ifndef KAL_PPSCANNER_H
#define KAL_PPSCANNER_H

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
	PREPROC_TOKEN_MACRO, PREPROC_TOKEN_UNMACRO,

	PREPROC_TOKEN_IDENTIFIER, PREPROC_TOKEN_REPLACEMENT,

	PREPROC_TOKEN_TEXT,

	PREPROC_TOKEN_ERROR, PREPROC_TOKEN_EOF
} PreprocTokenType;

typedef struct
{
	const char* start;
	PreprocTokenType type;
	int length;
	int line;
} PreprocToken;

typedef struct
{
	PreprocToken* tokens;
	size_t count;
	size_t capacity;
} PreprocTokenArray;

typedef struct
{
	const char* start;
	const char* current;
	int line;
	PreprocTokenArray tokenArray;
} PreprocScanner;

void initPreprocScanner(PreprocScanner* scanner, const char* source);
void freePreprocScanner(const PreprocScanner* scanner);

void preprocTokenize(PreprocScanner* scanner);

#endif //KAL_PPSCANNER_H
