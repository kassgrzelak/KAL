//
// Created by kassie on 12/03/2026.
//
#include "Compiler.h"

#include <ctype.h>
#include <stdio.h>

#define MAX_OPERAND_NUM 8

int getOffsetBank0(const uint8_t opcode)
{
	if (opcode >= OP0_NOP && opcode <= OP0_HLT)
		return 1;
	if (opcode >= OP0_OUT_C && opcode <= OP0_INC_P)
		return 2;
	if (opcode >= OP0_LD_RC && opcode <= OP0_LD_PP)
		return 3;

	return -1;
}

int getOffsetBank1(const uint8_t opcode)
{
	return -1;
}

typedef struct
{
	Bytecode* bytecode;
	size_t* jumpTable;

	const char* labelNames[256];
	int labelLengths[256];

	bool panicMode;
	bool hadError;

	int labelDeclsSeen;

	const Token* currentInstruction;
	const Token* operands[MAX_OPERAND_NUM];
	int operandNum;

	const Token* current;
} Compiler;

typedef void (*MnemonicParser)(Compiler*);

static void errorAt(Compiler* compiler, const Token* token, const char* message)
{
	if (compiler->panicMode) return;

	compiler->panicMode = true;
	compiler->hadError = true;

	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF)
		fprintf(stderr, " at end");
	else if (token->type == TOKEN_ERROR)
	{ /* Nothing. */ }
	else
		fprintf(stderr, " at '%.*s'", token->length, token->start);

	fprintf(stderr, ": %s\n", message);
}

static void noAddressingModeError(Compiler* compiler)
{
	errorAt(compiler, compiler->currentInstruction, "No addressing mode found matching given mnemonic and "
			"operands.");
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
		if (compiler->operandNum == MAX_OPERAND_NUM)
			return noAddressingModeError(compiler);

		compiler->operands[compiler->operandNum++] = peek(compiler);
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
	if (token->length >= 2 && token->start[0] == '0')
	{
		if (tolower(token->start[1]) == 'b')
			return strtol(token->start + 2, NULL, 2);
		if (tolower(token->start[1]) == 'x')
			return strtol(token->start + 2, NULL, 16);

		return strtol(token->start + 1, NULL, 8);
	}

	return strtol(token->start, NULL, 10);
}

static uint8_t parseOperand(Compiler* compiler, const int index)
{
	const Token* token = compiler->operands[index];

	if (token->type == TOKEN_LABEL_OPERAND)
		return parseLabelOperand(compiler, token);

	return parseNumberOperand(compiler, token);
}

static void parserNOP(Compiler* compiler)
{
	if (compiler->operandNum != 0)
		return noAddressingModeError(compiler);

	emitByte(compiler, OP0_NOP);
}

static void parserHLT(Compiler* compiler)
{
	if (compiler->operandNum != 0)
		return noAddressingModeError(compiler);

	emitByte(compiler, OP0_HLT);
}

static void parserOUT(Compiler* compiler)
{
	if (compiler->operandNum != 1)
		return noAddressingModeError(compiler);

	const TokenType operandType = compiler->operands[0]->type;

	if (operandType == TOKEN_CONSTANT)
	{
		emitByte(compiler, OP0_OUT_C);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_REGISTER)
	{
		emitByte(compiler, OP0_OUT_R);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_MEMORY)
	{
		emitByte(compiler, OP0_OUT_M);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_POINTER)
	{
		emitByte(compiler, OP0_OUT_P);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}

	noAddressingModeError(compiler);
}

static void parserINC(Compiler* compiler)
{
	if (compiler->operandNum != 1)
		return noAddressingModeError(compiler);

	const TokenType operandType = compiler->operands[0]->type;

	if (operandType == TOKEN_REGISTER)
	{
		emitByte(compiler, OP0_INC_R);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_MEMORY)
	{
		emitByte(compiler, OP0_INC_M);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_POINTER)
	{
		emitByte(compiler, OP0_INC_P);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}

	noAddressingModeError(compiler);
}

static void parserJMP(Compiler* compiler)
{
	if (compiler->operandNum != 1)
		return noAddressingModeError(compiler);

	const TokenType operandType = compiler->operands[0]->type;

	if (operandType == TOKEN_CONSTANT)
	{
		emitByte(compiler, OP0_JMP_C);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_REGISTER)
	{
		emitByte(compiler, OP0_JMP_R);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_MEMORY)
	{
		emitByte(compiler, OP0_JMP_M);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_POINTER)
	{
		emitByte(compiler, OP0_JMP_P);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}
	if (operandType == TOKEN_LABEL_OPERAND)
	{
		emitByte(compiler, OP0_JMP_L);
		emitByte(compiler, parseOperand(compiler, 0));
		return;
	}

	noAddressingModeError(compiler);
}

static void parserLD(Compiler* compiler)
{
	if (compiler->operandNum != 2)
		return noAddressingModeError(compiler);

	const TokenType operandType0 = compiler->operands[0]->type;
	const TokenType operandType1 = compiler->operands[1]->type;

	if (operandType0 == TOKEN_REGISTER)
	{
		if (operandType1 == TOKEN_CONSTANT)
		{
			emitByte(compiler, OP0_LD_RC);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_REGISTER)
		{
			emitByte(compiler, OP0_LD_RR);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_MEMORY)
		{
			emitByte(compiler, OP0_LD_RM);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_POINTER)
		{
			emitByte(compiler, OP0_LD_RP);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
	}
	if (operandType0 == TOKEN_MEMORY)
	{
		if (operandType1 == TOKEN_CONSTANT)
		{
			emitByte(compiler, OP0_LD_MC);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_REGISTER)
		{
			emitByte(compiler, OP0_LD_MR);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_MEMORY)
		{
			emitByte(compiler, OP0_LD_MM);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_POINTER)
		{
			emitByte(compiler, OP0_LD_MP);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
	}
	if (operandType0 == TOKEN_POINTER)
	{
		if (operandType1 == TOKEN_CONSTANT)
		{
			emitByte(compiler, OP0_LD_PC);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_REGISTER)
		{
			emitByte(compiler, OP0_LD_PR);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_MEMORY)
		{
			emitByte(compiler, OP0_LD_PM);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
		if (operandType1 == TOKEN_POINTER)
		{
			emitByte(compiler, OP0_LD_PP);
			emitByte(compiler, parseOperand(compiler, 0));
			emitByte(compiler, parseOperand(compiler, 1));
			return;
		}
	}

	noAddressingModeError(compiler);
}

const MnemonicParser mnemonicParserTable[] = {
	[TOKEN_NOP] = &parserNOP,
	[TOKEN_HLT] = &parserHLT,
	[TOKEN_OUT] = &parserOUT,
	[TOKEN_INC] = &parserINC,
	[TOKEN_JMP] = &parserJMP,
	[TOKEN_LD]  = &parserLD,
};

void parseInstruction(Compiler* compiler)
{
	return mnemonicParserTable[compiler->currentInstruction->type](compiler);
}

void statement(Compiler* compiler)
{
	const Token* nextToken = peek(compiler);
	advance(compiler);

	if (!isStatementStarter(nextToken->type))
		return errorAt(compiler, compiler->current, "Expected instruction mnemonic or label declaration at"
			"beginning of statement.");

	if (nextToken->type == TOKEN_LABEL_DECL)
	{
		compiler->jumpTable[compiler->labelDeclsSeen++] = compiler->bytecode->count;
		return;
	}

	compiler->currentInstruction = nextToken;
	compiler->operandNum = 0;
	consumeOperands(compiler);
	parseInstruction(compiler);
}

bool compile(Bytecode* bytecode, size_t* jumpTable, const char* source)
{
	Scanner scanner;
	initScanner(&scanner, source);
	tokenize(&scanner);

	Compiler compiler;
	compiler.bytecode = bytecode;
	compiler.jumpTable = jumpTable;
	compiler.panicMode = false;
	compiler.hadError = false;
	compiler.labelDeclsSeen = 0;
	compiler.currentInstruction = NULL;
	compiler.operandNum = 0;
	compiler.current = scanner.tokenArray.tokens;

	int labelsSeen = 0;

	for (size_t i = 0; i < scanner.tokenArray.count; ++i)
	{
		const Token token = scanner.tokenArray.tokens[i];

		if (token.type != TOKEN_LABEL_DECL)
			continue;
		if (labelsSeen == 256)
			return false; // TODO: Print error.

		compiler.labelNames[labelsSeen] = token.start;
		compiler.labelLengths[labelsSeen++] = token.length;
	}

	for (int i = 0; i < labelsSeen; ++i)
		printf("%.*s\n", compiler.labelLengths[i], compiler.labelNames[i]);

	while (peek(&compiler)->type != TOKEN_EOF)
	{
		statement(&compiler);

		if (compiler.panicMode)
			endPanic(&compiler);
	}

	// Add hlt instruction in case a label was placed at the end of a program with no instructions after.
	emitByte(&compiler, OP0_HLT);

	freeScanner(&scanner);
	return !compiler.hadError;
}
