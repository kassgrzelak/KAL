//
// Created by kassie on 16/03/2026.
//
#ifndef KAL_INSTRUCTIONS_H
#define KAL_INSTRUCTIONS_H

#include "Scanner.h"
#include "opcodes.inc"

typedef enum
{
#define X(opcode) \
	OP_##opcode,
	OPCODES_X
#undef X

	OPCODE_COUNT
} Opcode;

typedef struct
{
	uint8_t opcode;
	uint8_t length;
	uint16_t signature;
} InstrVariant;

static const InstrVariant nopVariants[] = {{OP_NOP, 1, 0}};

static const InstrVariant hltVariants[] = {{OP_HLT, 1, 0}};

static const InstrVariant outVariants[] = {
	{OP_OUT_C, 2, 01},
	{OP_OUT_R, 2, 02},
	{OP_OUT_M, 2, 03},
	{OP_OUT_P, 2, 04},
};

static const InstrVariant incVariants[] = {
	{OP_INC_R, 2, 02},
	{OP_INC_M, 2, 03},
	{OP_INC_P, 2, 04},
};

static const InstrVariant decVariants[] = {
	{OP_DEC_R, 2, 02},
	{OP_DEC_M, 2, 03},
	{OP_DEC_P, 2, 04},
};

static const InstrVariant jmpVariants[] = {
	{OP_JMP_C, 2, 01},
	{OP_JMP_R, 2, 02},
	{OP_JMP_M, 2, 03},
	{OP_JMP_P, 2, 04},
	{OP_JMP_L, 2, 05},
};

static const InstrVariant mvVariants[] = {
	{OP_MV_RC, 3, 021},
	{OP_MV_RR, 3, 022},
	{OP_MV_RM, 3, 023},
	{OP_MV_RP, 3, 024},

	{OP_MV_MC, 3, 031},
	{OP_MV_MR, 3, 032},
	{OP_MV_MM, 3, 033},
	{OP_MV_MP, 3, 034},

	{OP_MV_PC, 3, 041},
	{OP_MV_PR, 3, 042},
	{OP_MV_PM, 3, 043},
	{OP_MV_PP, 3, 044},
};

typedef struct
{
	TokenType token;
	const char* mnemonic;
	int operandCount;
	const InstrVariant* variants;
	int variantCount;
} InstrDef;

#define TABLE_ENTRY(nameCaps, nameLower, operandCount) \
	{TOKEN_##nameCaps, #nameLower, operandCount, nameLower##Variants, sizeof(nameLower##Variants) / sizeof(InstrVariant)},

static const InstrDef instrTable[] = {
	TABLE_ENTRY(NOP, nop, 0)
	TABLE_ENTRY(HLT, hlt, 0)
	TABLE_ENTRY(OUT, out, 1)
	TABLE_ENTRY(INC, inc, 1)
	TABLE_ENTRY(DEC, dec, 1)
	TABLE_ENTRY(JMP, jmp, 1)
	TABLE_ENTRY(MV, mv, 2)
};

#undef TABLE_ENTRY

static const uint8_t INSTR_COUNT =  sizeof(instrTable) / sizeof(InstrDef);

#endif // KAL_INSTRUCTIONS_H
