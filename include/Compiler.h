//
// Created by kassie on 12/03/2026.
//

#ifndef KAL_COMPILER_H
#define KAL_COMPILER_H

#include "Bytecode.h"
#include "Scanner.h"

// IMPORTANT: Checklist for adding a new instruction:
// - Add opcodes for all addressing modes to appropriate opcode enum in Compiler.h.
// - Add token to TokenType enum in Scanner.h.
// - Add scanning support in mnemonic() in Scanner.c.
// - Add debug printing support in printToken() in Scanner.c.

// Shorthand for instruction addressing modes:
// C - Constant.
// R - Register.
// M - RAM location.
// P - RAM pointer.
// L - Label.
typedef enum : uint8_t
{
	// Switch to second bank of instructions.
	OP0_BANK1,

	OP0_NOP,

	OP0_LD_RC, OP0_LD_RR, OP0_LD_RM, OP0_LD_RP,
	OP0_LD_MC, OP0_LD_MR, OP0_LD_MM, OP0_LD_MP,
	OP0_LD_PC, OP0_LD_PR, OP0_LD_PM, OP0_LD_PP,

	OP0_OUT_C, OP0_OUT_R, OP0_OUT_M, OP0_OUT_P,

	OP0_INC_R, OP0_INC_M, OP0_INC_P,

	OP0_JMP_C, OP0_JMP_R, OP0_JMP_M, OP0_JMP_P, OP0_JMP_L,

	OP0_HLT,
} OpcodeBank0;

typedef enum : uint8_t
{
	OP1_BANK_2,
} OpcodeBank1;

typedef struct
{
	Bytecode* bytecode;
	size_t* jumpTable;

	const char* labelNames[256];
	int labelLengths[256];

	Token* current;
} Compiler;

void initCompiler(Compiler* compiler, Bytecode* bytecode, size_t* jumpTable);

bool compile(Compiler* compiler, const char* source);

#endif //KAL_COMPILER_H