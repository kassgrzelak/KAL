//
// Created by kassie on 12/03/2026.
//
#include "Compiler.h"

#include <ctype.h>
#include <stdio.h>

#include "instructions.h"

AddressingMode tokenTypeToAddressingMode(const TokenType type)
{
	switch (type) {
	case TOKEN_CONSTANT:
		return AM_CONST;
	case TOKEN_REGISTER:
		return AM_REG;
	case TOKEN_MEMORY:
		return AM_MEM;
	case TOKEN_POINTER:
		return AM_PTR;
	case TOKEN_LABEL_OPERAND:
		return AM_LABEL;

	default:
		return AM_NONE;
	}
}

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

	const char* memNames[256];
	int memNameLengths[256];

	bool panicMode;
	bool hadError;

	int labelDeclsSeen;

	const Token* currentInstruction;
	const Token* operands[MAX_OPERAND_NUM];
	int operandCount;

	const Token* current;
} Compiler;

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

static const Token* peek(const Compiler* compiler)
{
	return compiler->current;
}

static void advance(Compiler* compiler)
{
	for (;;)
	{
		++compiler->current;
		if (compiler->current->type != TOKEN_ERROR)
			break;

		errorAt(compiler, compiler->current, compiler->current->start);
	}
}

static void endPanic(Compiler* compiler)
{
	while (!isStatementStarter(peek(compiler)->type) && peek(compiler)->type != TOKEN_EOF)
		advance(compiler);

	compiler->panicMode = false;
}

void consumeOperands(Compiler* compiler)
{
	while (isOperand(peek(compiler)->type))
	{
		if (compiler->operandCount == MAX_OPERAND_NUM)
			return noAddressingModeError(compiler);

		compiler->operands[compiler->operandCount++] = peek(compiler);
		advance(compiler);
	}
}

static void emitByte(const Compiler* compiler, const uint8_t byte)
{
	writeBytecode(compiler->bytecode, byte);
}

static uint8_t parseLabelOperand(Compiler* compiler, const Token* token)
{
	for (int i = 0; i < 256; ++i)
	{
		if (compiler->labelNames[i] == NULL)
			continue;
		if (compiler->labelLengths[i] != token->length)
			continue;

		bool notEqual = false;

		for (int charIndex = 0; charIndex < token->length; ++charIndex)
			if (tolower(compiler->labelNames[i][charIndex]) != tolower(token->start[charIndex]))
			{
				notEqual = true;
				break;
			}

		if (notEqual)
			continue;

		return i;
	}

	errorAt(compiler, token, "Label operand does not refer to an existing label.");
	return 0;
}

static uint8_t parseNumberOperand(Compiler* compiler, const Token* token)
{
	if (!isdigit(token->start[0]))
		warningAt(compiler, token, "Attempted to parse non-number token in parseNumberOperand(). This is a "
			"compiler bug, not your fault.");

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

static uint8_t parseConstant(Compiler* compiler, const Token* token)
{
	if (isDigit(token->start[0], BASE_DECIMAL))
		return parseNumberOperand(compiler, token);

	for (int i = 0; i < 256; ++i)
	{
		if (compiler->memNames[i] == NULL)
			continue;
		if (compiler->memNameLengths[i] != token->length)
			continue;

		bool notEqual = false;

		for (int charIndex = 0; charIndex < token->length; ++charIndex)
			if (tolower(compiler->memNames[i][charIndex]) != tolower(token->start[charIndex]))
			{
				notEqual = true;
				break;
			}

		if (notEqual)
			continue;

		return i;
	}

	errorAt(compiler, token, "Named RAM location name does not match any name given in any #namedmem directive"
		" in the program.");
	return 0;
}

static uint8_t parseMemoryOperand(Compiler* compiler, const Token* token)
{
	if (isDigit(token->start[0], BASE_DECIMAL))
		return parseNumberOperand(compiler, token);

	for (int i = 0; i < 256; ++i)
	{
		if (compiler->memNames[i] == NULL)
			continue;
		if (compiler->memNameLengths[i] != token->length)
			continue;

		bool notEqual = false;

		for (int charIndex = 0; charIndex < token->length; ++charIndex)
			if (tolower(compiler->memNames[i][charIndex]) != tolower(token->start[charIndex]))
			{
				notEqual = true;
				break;
			}

		if (notEqual)
			continue;

		return i;
	}

	errorAt(compiler, token, "Named RAM location name does not match any name given in any #namedmem directive"
		" in the program.");
	return 0;
}

static uint8_t parseOperand(Compiler* compiler, const int index)
{
	const Token* token = compiler->operands[index];

	if (token->type == TOKEN_LABEL_OPERAND)
		return parseLabelOperand(compiler, token);
	if (token->type == TOKEN_MEMORY)
		return parseMemoryOperand(compiler, token);
	if (token->type == TOKEN_CONSTANT)
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

static void checkOperandValue(Compiler* compiler, const uint8_t value, const Token* token)
{
	if (token->type == TOKEN_REGISTER)
		if (value >= 8)
			errorAt(compiler, token, "Invalid register index.");
	if (token->type == TOKEN_POINTER)
		if (value >= 8)
			errorAt(compiler, token, "Invalid pointer index.");
}

static void parseInstruction(Compiler* compiler)
{
	const InstrDef* def = NULL;

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

		checkOperandValue(compiler, operand, compiler->operands[i]);

#ifdef DEBUG_PRINT
		printOperand(operand);
#endif

		emitByte(compiler, operand);
	}
}

static void dataFromDirective(Compiler* compiler)
{
	if (peek(compiler)->type != TOKEN_MEMORY)
		return errorAt(compiler, compiler->current, "Expected ram index after #datafrom directive.");

	int ramIndex = parseMemoryOperand(compiler, peek(compiler));
	advance(compiler);

	int elementNum = 0;

	for (TokenType type = compiler->current[elementNum].type; type == TOKEN_CONSTANT;)
		type = compiler->current[++elementNum].type;


	if (ramIndex + elementNum - 1 > 255)
		return errorAt(compiler, compiler->current, "Number of elements in #datafrom directive exceed the"
			" space in RAM.");

	while (peek(compiler)->type == TOKEN_CONSTANT)
	{
		const uint8_t value = parseConstant(compiler, peek(compiler));
		compiler->ram[ramIndex++] = value;
		advance(compiler);
	}
}

static void dataToDirective(Compiler* compiler)
{
	if (peek(compiler)->type != TOKEN_MEMORY)
		return errorAt(compiler, compiler->current, "Expected ram index after #datafrom directive.");

	int ramIndex = parseMemoryOperand(compiler, peek(compiler));
	advance(compiler);

	int elementNum = 0;

	for (TokenType type = compiler->current[elementNum].type; type == TOKEN_CONSTANT;)
		type = compiler->current[++elementNum].type;


	if (ramIndex - elementNum + 1 < 0)
		return errorAt(compiler, compiler->current, "Number of elements in #datato directive exceed the"
			" space in RAM.");

	ramIndex -= elementNum - 1;

	while (peek(compiler)->type == TOKEN_CONSTANT)
	{
		const uint8_t value = parseConstant(compiler, peek(compiler));
		compiler->ram[ramIndex++] = value;
		advance(compiler);
	}
}

static void namedMemDirective(Compiler* compiler)
{
	if (peek(compiler)->type != TOKEN_MEMORY)
		return errorAt(compiler, compiler->current, "Expected memory location name after #namedmem directive.");


}

static void statement(Compiler* compiler)
{
	const Token* nextToken = peek(compiler);
	advance(compiler);

	if (!isStatementStarter(nextToken->type))
		return errorAt(compiler, compiler->current - 1, "Expected instruction mnemonic or label "
			"declaration at beginning of statement.");

	if (nextToken->type == TOKEN_DATAFROM)
		return dataFromDirective(compiler);
	if (nextToken->type == TOKEN_DATATO)
		return dataToDirective(compiler);
	if (nextToken->type == TOKEN_NAMEDMEM)
		return namedMemDirective(compiler);

	if (nextToken->type == TOKEN_LABEL_DECL)
	{
		compiler->jumpTable[compiler->labelDeclsSeen++] = compiler->bytecode->count;
		return;
	}

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
static void printMemNames(const Compiler* compiler)
{
	printf("\n=== NAMED RAM ADDRESSES ===\n");
	printf("Address | Address name\n");
	printf("-----------------------\n");

	for (int i = 0; i < 256; ++i)
	{
		if (compiler->memNames[i] == NULL)
			continue;

		printf("0x%02x    | '%.*s'\n", i, compiler->memNameLengths[i], compiler->memNames[i]);
	}
}
#endif

bool compile(Bytecode* bytecode, size_t* jumpTable, uint8_t* ram, const char* source)
{
	Scanner scanner;
	initScanner(&scanner, source);
	tokenize(&scanner);

	Compiler compiler;
	compiler.bytecode = bytecode;
	compiler.ram = ram;
	compiler.jumpTable = jumpTable;
	for (int i = 0; i < 256; ++i)
		compiler.labelNames[i] = NULL;
	for (int i = 0; i < 256; ++i)
		compiler.memNames[i] = NULL;
	compiler.panicMode = false;
	compiler.hadError = false;
	compiler.labelDeclsSeen = 0;
	compiler.currentInstruction = NULL;
	compiler.operandCount = 0;
	compiler.current = scanner.tokenArray.tokens;

	// Label declaration pass.
	int labelsSeen = 0;

	for (size_t i = 0; i < scanner.tokenArray.count; ++i)
	{
		const Token token = scanner.tokenArray.tokens[i];

		if (token.type != TOKEN_LABEL_DECL)
			continue;
		if (labelsSeen == 256)
		{
			errorAt(&compiler, &token, "Exceeded the limit of 256 labels in a single program. Compilation "
				"aborted.");
			freeScanner(&scanner);
			return false;
		}

		compiler.labelNames[labelsSeen] = token.start;
		compiler.labelLengths[labelsSeen++] = token.length;
	}

	bool namedMemSeen = false;

	// Named RAM locations pass.
	for (size_t i = 0; i < scanner.tokenArray.count; ++i)
	{
		Token* directiveToken = &scanner.tokenArray.tokens[i];

		if (directiveToken->type != TOKEN_NAMEDMEM)
			continue;

		namedMemSeen = true;

		if (scanner.tokenArray.count - 1 - i < 2)
		{
			errorAt(&compiler, directiveToken, "Expected memory index and name after #namedmem directive. "
				"Compilation aborted.");
			freeScanner(&scanner);
			return false;
		}
		if (directiveToken[1].type != TOKEN_MEMORY)
		{
			errorAt(&compiler, &directiveToken[1], "Expected memory name and index after #namedmem directive. "
				"Compilation aborted.");
			freeScanner(&scanner);
			return false;
		}
		if (!isAlpha(directiveToken[1].start[0]))
		{
			errorAt(&compiler, &directiveToken[1], "Expected memory name in #namedmem directive to be an "
				"identifier. Compilation aborted.");
			freeScanner(&scanner);
			return false;
		}
		if (directiveToken[2].type != TOKEN_MEMORY)
		{
			errorAt(&compiler, &directiveToken[2], "Expected memory name and index after #namedmem directive. "
				"Compilation aborted.");
			freeScanner(&scanner);
			return false;
		}
		if (!isDigit(directiveToken[2].start[0], BASE_DECIMAL))
		{
			errorAt(&compiler, &directiveToken[2], "Expected memory index in #namedmem directive to be a "
				"number. Compilation aborted.");
			freeScanner(&scanner);
			return false;
		}

		const uint8_t memIndex = parseNumberOperand(&compiler, &directiveToken[2]);

		if (compiler.memNames[memIndex] != NULL)
			warningAt(&compiler, directiveToken, "Renaming ");

		compiler.memNames[memIndex] = directiveToken[1].start;
		compiler.memNameLengths[memIndex] = directiveToken[1].length;

		const Token skipToken = {NULL, TOKEN_SKIP, 0, 0};
		directiveToken[0] = skipToken;
		directiveToken[1] = skipToken;
		directiveToken[2] = skipToken;
	}

#ifdef DEBUG_PRINT
	if (labelsSeen > 0)
		printLabelDecls(&compiler, labelsSeen);
	if (namedMemSeen)
		printMemNames(&compiler);
#endif

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
#endif
	emitByte(&compiler, OP_HLT);

	freeScanner(&scanner);
	return !compiler.hadError;
}
