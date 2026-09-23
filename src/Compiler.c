//
// Created by kassie on 12/03/2026.
//
#include "Compiler.h"

#include <ctype.h>
#include <stdio.h>

#include "instructions.h"

// Convert TokenType enum to AddressingMode enum.
AddressingMode tokenTypeToAddressingMode(const TokenType type)
{
	switch (type) {
	case TOKEN_CONSTANT:
	case TOKEN_STRING_CHAR:
	case TOKEN_ALIAS_ADDRESS:
		return AM_CONST;
	case TOKEN_REGISTER:
		return AM_REG;
	case TOKEN_MEMORY:
	case TOKEN_MEMORY_ALIAS:
		return AM_MEM;
	case TOKEN_POINTER:
		return AM_PTR;
	case TOKEN_LABEL_OPERAND:
		return AM_LABEL;

	default:
		return AM_NONE;
	}
}

// Calculate 16-bit addressing mode signature from individual operand types.
uint16_t signatureFromModes(const AddressingMode* modes, const int count)
{
	uint16_t signature = 0;

	for (int i = 0; i < count; ++i)
		signature |= modes[count - i - 1] << (3 * i);

	return signature;
}

#define MAX_OPERAND_NUM 8

typedef struct
{
	Bytecode* bytecode;
	uint8_t* ram;
	size_t* jumpTable;

	const char* labelNames[256];
	int labelLengths[256];

	const char* memAliases[256];
	int memAliasLengths[256];

	bool panicMode;
	bool hadError;

	int labelDeclsSeen;

	const Token* currentInstruction;
	const Token* operands[MAX_OPERAND_NUM];
	int operandCount;

	const Token* current;
} Compiler;

static bool isAddressingModeType(const Token* token, const AddressingMode operandType)
{
	switch (token->type)
	{
	case TOKEN_CONSTANT:
	case TOKEN_STRING_CHAR:
	case TOKEN_ALIAS_ADDRESS:
		return operandType == AM_CONST;
	case TOKEN_REGISTER:
		return operandType == AM_REG;
	case TOKEN_MEMORY:
	case TOKEN_MEMORY_ALIAS:
		return operandType == AM_MEM;
	case TOKEN_POINTER:
		return operandType == AM_PTR;
	case TOKEN_LABEL_OPERAND:
		return operandType == AM_LABEL;

	default:
		return false;
	}
}

static void errorAt(Compiler* compiler, const Token* token, const char* message)
{
	if (compiler->panicMode) return;

	compiler->panicMode = true;
	compiler->hadError = true;

	printf("[line %d] Error", token->line);

	if (token->type == TOKEN_EOF)
		printf(" at end");
	else if (token->type == TOKEN_ERROR)
	{ /* Nothing. */ }
	else
		printf(" at '%.*s'", token->length, token->start);

	printf(": %s\n", message);
}

static void noInstructionError(Compiler* compiler)
{
	errorAt(compiler, compiler->currentInstruction, "No instruction found matching this token. This means "
		"the dev forgot to add an entry to instrTable. Not your fault!");
}

static void noAddressingModeError(Compiler* compiler)
{
	errorAt(compiler, compiler->currentInstruction, "No addressing mode found matching given mnemonic and "
			"operands.");
}

static void warningAt(Compiler* compiler, const Token* token, const char* message)
{
	printf("[line %d] Warning", token->line);

	if (token->type == TOKEN_EOF)
		printf(" at end");
	else if (token->type == TOKEN_ERROR)
	{ /* Nothing. */ }
	else
		printf(" at '%.*s'", token->length, token->start);

	printf(": %s\n", message);
}

// Get the next token but don't consume it.
static const Token* peek(const Compiler* compiler)
{
	return compiler->current;
}

// Consume the next token.
static void advance(Compiler* compiler)
{
	// Long strings of error tokens must be moved past.
	for (;;)
	{
		++compiler->current;
		if (compiler->current->type != TOKEN_ERROR)
			break;

		errorAt(compiler, compiler->current, compiler->current->start);
	}
}

// Consume tokens until a safe starting point to try compiling again is found.
static void endPanic(Compiler* compiler)
{
	while (!isStatementStarterType(peek(compiler)->type) && peek(compiler)->type != TOKEN_EOF)
		advance(compiler);

	compiler->panicMode = false;
}

// Consume all the following operand-type tokens and add them to the operand stack.
void consumeOperands(Compiler* compiler)
{
	while (isOperandType(peek(compiler)->type))
	{
		if (compiler->operandCount == MAX_OPERAND_NUM)
			return noAddressingModeError(compiler);

		compiler->operands[compiler->operandCount++] = peek(compiler);
		advance(compiler);
	}
}

// Add a byte to the end of the bytecode array.
static void emitByte(const Compiler* compiler, const uint8_t byte)
{
	writeBytecode(compiler->bytecode, byte);
}

// Find the index of a case-insensitive name in an indexed name array. Returns -1 if no match is found.
static int findName(const char* const names[256], const int lengths[256], const Token* token)
{
	for (int i = 0; i < 256; ++i)
	{
		if (names[i] == NULL)
			continue;
		if (lengths[i] != token->length)
			continue;

		bool notEqual = false;

		for (int charIndex = 0; charIndex < token->length; ++charIndex)
			if (tolower(names[i][charIndex]) != tolower(token->start[charIndex]))
			{
				notEqual = true;
				break;
			}

		if (!notEqual)
			return i;
	}

	return -1;
}

// Parse a label operand token and return its index in the labelNames array.
static uint8_t parseLabelOperand(Compiler* compiler, const Token* token)
{
	// Iterate over all label declarations to see if a match is found.
	const int labelIndex = findName(compiler->labelNames, compiler->labelLengths, token);
	if (labelIndex != -1)
		return labelIndex;

	errorAt(compiler, token, "Label operand does not refer to an existing label.");
	return 0;
}

// Parse a numerical operand token (constant, memory, or pointer) and return its numerical value.
// The token passed to this function MUST be numerical.
static uint8_t parseNumberOperand(Compiler* compiler, const Token* token)
{
	if (!isdigit(token->start[0]))
		warningAt(compiler, token, "Attempted to parse non-number token in parseNumberOperand(). This is a "
			"compiler bug, not your fault.");

	// Parse base specifier if present.
	if (token->length >= 2 && token->start[0] == '0')
	{
		if (tolower(token->start[1]) == 'b')
		{
			const long number = strtol(token->start + 2, NULL, 2);
			if (number > 255)
				warningAt(compiler, token, "Truncated integer literal.");
			return number;
		}
		if (tolower(token->start[1]) == 'x')
		{
			const long number = strtol(token->start + 2, NULL, 16);
			if (number > 255)
				warningAt(compiler, token, "Truncated integer literal.");
			return number;
		}

		const long number = strtol(token->start + 1, NULL, 8);
		if (number > 255)
			warningAt(compiler, token, "Truncated integer literal.");
		return number;
	}

	const long number = strtol(token->start, NULL, 10);
	if (number > 255)
		warningAt(compiler, token, "Truncated integer literal.");
	return number;
}

static uint8_t parseStringChar(Compiler* compiler, const Token* token)
{
	if (token->start[0] == '\\')
	{
		switch (token->start[1])
		{
		case '0':
			return '\0';
		case 'a':
			return '\a';
		case 'b':
			return '\b';
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'v':
			return '\v';

		default:
			return token->start[1];
		}
	}

	return token->start[0];
}

// Parse a constant token (constant or string char) and return its numerical value.
static uint8_t parseConstant(Compiler* compiler, const Token* token)
{
	if (token->type == TOKEN_STRING_CHAR)
		return parseStringChar(compiler, token);

	if (token->type == TOKEN_CONSTANT)
		return parseNumberOperand(compiler, token);

	// Else alias address.

	const int memIndex = findName(compiler->memAliases, compiler->memAliasLengths, token);
	if (memIndex != -1)
		return memIndex;

	errorAt(compiler, token, "Aliased RAM location name does not match any name given in any #memalias directive"
		" in the program.");
	return 0;
}

// Parse a RAM operand and return the numerical address it refers to.
static uint8_t parseMemoryOperand(Compiler* compiler, const Token* token)
{
	if (token->type == TOKEN_MEMORY)
		return parseNumberOperand(compiler, token);

	// Else aliased mem address.

	const int memIndex = findName(compiler->memAliases, compiler->memAliasLengths, token);
	if (memIndex != -1)
		return memIndex;

	errorAt(compiler, token, "Aliased RAM location name does not match any name given in any #memalias directive"
		" in the program.");
	return 0;
}

// Parse an operand and return its numerical value.
static uint8_t parseOperand(Compiler* compiler, const int index)
{
	const Token* token = compiler->operands[index];

	if (isAddressingModeType(token, AM_LABEL))
		return parseLabelOperand(compiler, token);
	if (isAddressingModeType(token, AM_MEM))
		return parseMemoryOperand(compiler, token);
	if (isAddressingModeType(token, AM_CONST))
		return parseConstant(compiler, token);

	return parseNumberOperand(compiler, token);
}

#ifdef DEBUG_PRINT
static void printOpcode(const size_t byte, const uint8_t opcode)
{
	printf("0x%04llx | ", byte);

	switch (opcode)
	{
#define X(opcode) \
	case OP_##opcode: printf("OP_%-12s |\n", #opcode); break;
		OPCODES_X
#undef X

	default:
		printf("%-9s |\n", "UNKNOWN");
	}
}
#endif

#ifdef DEBUG_PRINT
static void printOperand(const uint8_t operand)
{
	printf("       |                 | %d\n", operand);
}
#endif

// Bounds-check register and pointer token indices.
static void checkRegisterPointerIndex(Compiler* compiler, const uint8_t value, const Token* token)
{
	if (token->type == TOKEN_REGISTER)
		if (value >= 8)
			errorAt(compiler, token, "Invalid register index.");
	if (token->type == TOKEN_POINTER)
		if (value >= 8)
			errorAt(compiler, token, "Invalid pointer index.");
}

// Having consumed an instruction token and all the following operand tokens, decide which opcode they correspond to and
// emit the appropriate bytes.
static void parseInstruction(Compiler* compiler)
{
	const InstrDef* def = NULL;

	// Search instruction table for an instruction matching the token type.
	for (int i = 0; i < INSTR_COUNT; ++i)
		if (instrTable[i].token == compiler->currentInstruction->type)
		{
			def = &instrTable[i];
			break;
		}

	if (!def)
		return noInstructionError(compiler);

	if (compiler->operandCount != def->operandCount)
		return noAddressingModeError(compiler);

	AddressingMode modes[MAX_OPERAND_NUM];
	for (int i = 0; i < def->operandCount; ++i)
		modes[i] = tokenTypeToAddressingMode(compiler->operands[i]->type);

	const uint16_t signature = signatureFromModes(modes, def->operandCount);

	const InstrVariant* variant = NULL;

	// Search variants array for a variant matching the same operand types as were given.
	for (int i = 0; i < def->variantCount; ++i)
		if (def->variants[i].signature == signature)
		{
			variant = &def->variants[i];
			break;
		}

	if (!variant)
		return noAddressingModeError(compiler);

#ifdef DEBUG_PRINT
	printOpcode(compiler->bytecode->count, variant->opcode);
#endif

	emitByte(compiler, variant->opcode);

	for (int i = 0; i < compiler->operandCount; ++i)
	{
		const uint8_t operand = parseOperand(compiler, i);

		checkRegisterPointerIndex(compiler, operand, compiler->operands[i]);

#ifdef DEBUG_PRINT
		printOperand(operand);
#endif

		emitByte(compiler, operand);
	}
}

static void dataFromDirective(Compiler* compiler)
{
	if (!isAddressingModeType(peek(compiler), AM_MEM))
		return errorAt(compiler, compiler->current, "Expected ram address after #datafrom directive.");

	int ramIndex = parseMemoryOperand(compiler, peek(compiler));
	advance(compiler);

	// Get element count for bounds-checking.
	int elementCount = 0;
	for (; isAddressingModeType(&compiler->current[elementCount], AM_CONST); ++elementCount)
	{}

	if (ramIndex + elementCount - 1 > 255)
		return errorAt(compiler, compiler->current, "Number of elements in #datafrom directive exceed the"
			" space in RAM.");

	while (isAddressingModeType(peek(compiler), AM_CONST))
	{
		const uint8_t value = parseConstant(compiler, peek(compiler));
		compiler->ram[ramIndex++] = value;
		advance(compiler);
	}
}

static void dataToDirective(Compiler* compiler)
{
	if (!isAddressingModeType(peek(compiler), AM_MEM))
		return errorAt(compiler, compiler->current, "Expected ram address after #datafrom directive.");

	int ramIndex = parseMemoryOperand(compiler, peek(compiler));
	advance(compiler);

	// Get element count for bounds-checking.
	int elementCount = 0;
	for (; isAddressingModeType(&compiler->current[elementCount], AM_CONST); ++elementCount)
	{}

	if (ramIndex - elementCount + 1 < 0)
		return errorAt(compiler, compiler->current, "Number of elements in #datato directive exceed the"
			" space in RAM.");

	ramIndex -= elementCount - 1;

	while (isAddressingModeType(peek(compiler), AM_CONST))
	{
		const uint8_t value = parseConstant(compiler, peek(compiler));
		compiler->ram[ramIndex++] = value;
		advance(compiler);
	}
}

// Compile the next statement in the token array.
static void statement(Compiler* compiler)
{
	const Token* nextToken = peek(compiler);
	advance(compiler);

	if (!isStatementStarterType(nextToken->type))
		return errorAt(compiler, compiler->current - 1, "Expected instruction mnemonic or label "
			"declaration at beginning of statement.");

	if (nextToken->type == TOKEN_DATAFROM)
		return dataFromDirective(compiler);
	if (nextToken->type == TOKEN_DATATO)
		return dataToDirective(compiler);

	if (nextToken->type == TOKEN_LABEL_DECL)
	{
		compiler->jumpTable[compiler->labelDeclsSeen++] = compiler->bytecode->count;
		return;
	}

	// Else, instruction mnemonic.

	compiler->currentInstruction = nextToken;
	compiler->operandCount = 0;
	consumeOperands(compiler);
	parseInstruction(compiler);
}

#ifdef DEBUG_PRINT
static void printLabelDecls(const Compiler* compiler, const int labelsSeen)
{
	printf("\n=== LABEL DECLARATIONS ===\n");
	printf("Index | Label name\n");
	printf("-------------------\n");

	for (int i = 0; i < labelsSeen; ++i)
		printf("%03d   | '%.*s'\n", i, compiler->labelLengths[i], compiler->labelNames[i]);
}
#endif

#ifdef DEBUG_PRINT
static void printMemAliases(const Compiler* compiler)
{
	printf("\n=== ALIASED RAM ADDRESSES ===\n");
	printf("Address | Address alias\n");
	printf("-----------------------\n");

	for (int i = 0; i < 256; ++i)
	{
		if (compiler->memAliases[i] == NULL)
			continue;

		printf("0x%02x    | '%.*s'\n", i, compiler->memAliasLengths[i], compiler->memAliases[i]);
	}
}
#endif

static void initCompiler(Compiler* compiler, const Scanner* scanner,
	Bytecode* bytecode, size_t* jumpTable, uint8_t* ram)
{
	compiler->bytecode = bytecode;
	compiler->ram = ram;
	compiler->jumpTable = jumpTable;

	for (int i = 0; i < 256; ++i)
		compiler->labelNames[i] = NULL;
	for (int i = 0; i < 256; ++i)
		compiler->memAliases[i] = NULL;

	compiler->panicMode = false;
	compiler->hadError = false;
	compiler->labelDeclsSeen = 0;
	compiler->currentInstruction = NULL;
	compiler->operandCount = 0;
	compiler->current = scanner->tokenArray.tokens;
}

static bool labelDeclPass(Compiler* compiler, const Scanner* scanner)
{
	int labelsSeen = 0;

	for (size_t i = 0; i < scanner->tokenArray.count; ++i)
	{
		const Token token = scanner->tokenArray.tokens[i];

		if (token.type != TOKEN_LABEL_DECL)
			continue;
		if (labelsSeen == 256)
		{
			errorAt(compiler, &token, "Exceeded the limit of 256 labels in a single program. Compilation "
				"aborted.");
			return false;
		}

		compiler->labelNames[labelsSeen] = token.start;
		compiler->labelLengths[labelsSeen++] = token.length;
	}

#ifdef DEBUG_PRINT
	if (labelsSeen > 0)
		printLabelDecls(compiler, labelsSeen);
#endif

	return true;
}

static bool memAliasPass(Compiler* compiler, Scanner* scanner)
{
#ifdef DEBUG_PRINT
	bool memAliasSeen = false;
#endif

	for (size_t i = 0; i < scanner->tokenArray.count; ++i)
	{
		Token* directiveToken = &scanner->tokenArray.tokens[i];

		if (directiveToken->type != TOKEN_MEMALIAS)
			continue;

#ifdef DEBUG_PRINT
		memAliasSeen = true;
#endif

		if (scanner->tokenArray.count - 1 - i < 2)
		{
			errorAt(compiler, directiveToken, "Expected memory name and address after #memalias directive. "
				"Compilation aborted.");
			return false;
		}
		if (directiveToken[1].type != TOKEN_MEMORY_ALIAS)
		{
			errorAt(compiler, &directiveToken[1], "Expected memory name and address after #memalias directive. "
				"Compilation aborted.");
			return false;
		}
		if (directiveToken[2].type != TOKEN_MEMORY)
		{
			errorAt(compiler, &directiveToken[2], "Expected memory name and address after #memalias directive. "
				"Compilation aborted.");
			return false;
		}

		const uint8_t memAddress = parseNumberOperand(compiler, &directiveToken[2]);

		if (compiler->memAliases[memAddress] != NULL)
			warningAt(compiler, directiveToken, "Re-aliasing already aliased RAM address.");

		compiler->memAliases[memAddress] = directiveToken[1].start;
		compiler->memAliasLengths[memAddress] = directiveToken[1].length;

		const Token skipToken = {NULL, TOKEN_SKIP, 0, 0};
		directiveToken[0] = skipToken;
		directiveToken[1] = skipToken;
		directiveToken[2] = skipToken;
	}

#ifdef DEBUG_PRINT
	if (memAliasSeen)
		printMemAliases(compiler);
#endif

	return true;
}

// Compile source code into bytecode.
bool compile(Bytecode* bytecode, size_t* jumpTable, uint8_t* ram, const char* source)
{
	Scanner scanner;
	initScanner(&scanner, source);
	tokenize(&scanner);

	Compiler compiler;
	initCompiler(&compiler, &scanner, bytecode, jumpTable, ram);

	if (!labelDeclPass(&compiler, &scanner))
	{
		freeScanner(&scanner);
		return false;
	}
	if (!memAliasPass(&compiler, &scanner))
	{
		freeScanner(&scanner);
		return false;
	}

#ifdef DEBUG_PRINT
	printf("\n=== BYTECODE ===\n");
	printf("byte   | Opcode          | Operands\n");
	printf("------------------------------\n");
#endif

	if (peek(&compiler)->type == TOKEN_ERROR)
		errorAt(&compiler, peek(&compiler), peek(&compiler)->start);

	while (peek(&compiler)->type != TOKEN_EOF)
	{
		if (peek(&compiler)->type == TOKEN_SKIP)
		{
			advance(&compiler);
			continue;
		}

		statement(&compiler);

		if (compiler.panicMode)
			endPanic(&compiler);
	}

	// Add hlt instruction in case a label was placed at the end of a program with no instructions after.
#ifdef DEBUG_PRINT
	printOpcode(compiler.bytecode->count, OP_HLT);
	printf("\n");
#endif
	emitByte(&compiler, OP_HLT);

	freeScanner(&scanner);
	return !compiler.hadError;
}
