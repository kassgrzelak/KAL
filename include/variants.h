//
// Created by kassie on 17/03/2026.
//
#ifndef KAL_VARIANTS_INC
#define KAL_VARIANTS_INC

static const InstrVariant nopVariants[] = {{OP_NOP, 1}};

static const InstrVariant hltVariants[] = {{OP_HLT, 1}};

static const InstrVariant callVariants[] = {{OP_CALL_L, 2}};

static const InstrVariant retVariants[] = {{OP_RET, 1}};

static const InstrVariant jmpVariants[] = {
	{OP_JMP_C, 01},
	{OP_JMP_R, 02},
	{OP_JMP_M, 03},
	{OP_JMP_P, 04},
	{OP_JMP_L, 05},
};

static const InstrVariant jmpzVariants[] = {
	{OP_JMPZ_RC, 021},
	{OP_JMPZ_RR, 022},
	{OP_JMPZ_RM, 023},
	{OP_JMPZ_RP, 024},
	{OP_JMPZ_RL, 025},
	{OP_JMPZ_MC, 031},
	{OP_JMPZ_MR, 032},
	{OP_JMPZ_MM, 033},
	{OP_JMPZ_MP, 034},
	{OP_JMPZ_ML, 035},
	{OP_JMPZ_PC, 041},
	{OP_JMPZ_PR, 042},
	{OP_JMPZ_PM, 043},
	{OP_JMPZ_PP, 044},
	{OP_JMPZ_PL, 045},
};

static const InstrVariant jmpnzVariants[] = {
	{OP_JMPNZ_RC, 021},
	{OP_JMPNZ_RR, 022},
	{OP_JMPNZ_RM, 023},
	{OP_JMPNZ_RP, 024},
	{OP_JMPNZ_RL, 025},
	{OP_JMPNZ_MC, 031},
	{OP_JMPNZ_MR, 032},
	{OP_JMPNZ_MM, 033},
	{OP_JMPNZ_MP, 034},
	{OP_JMPNZ_ML, 035},
	{OP_JMPNZ_PC, 041},
	{OP_JMPNZ_PR, 042},
	{OP_JMPNZ_PM, 043},
	{OP_JMPNZ_PP, 044},
	{OP_JMPNZ_PL, 045},
};

#include "generated/generatedVariants.h"

#endif // KAL_VARIANTS_INC
