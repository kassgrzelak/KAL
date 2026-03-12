//
// Created by kassie on 12/03/2026.
//

#ifndef KAL_COMPILER_H
#define KAL_COMPILER_H

#include "Bytecode.h"
#include "Scanner.h"

// Shorthand for instruction addressing modes:
// C - Constant.
// R - Register.
// M - RAM location.
// P - RAM pointer.
// L - Label.
typedef enum : uint8_t
{
	// Switch to second bank of instructions.
	BANK_1,

	NOP,

	LD_RC, LD_RR, LD_RM, LD_RP,
	LD_MC, LD_MR, LD_MM, LD_MP,
	LD_PC, LD_PR, LD_PM, LD_PP,

	OUT_C, OUT_R, OUT_M, OUT_P,

	JMP_C, JMP_R, JMP_M, JMP_P, JMP_L,
} OpcodeBank0;

typedef enum : uint8_t
{
	BANK_2,
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