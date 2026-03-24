//
// Created by kassie on 17/03/2026.
//
#ifndef KAL_VARIANTS_INC
#define KAL_VARIANTS_INC

static const InstrVariant nopVariants[] = {{OP_NOP, 1, 0}};

static const InstrVariant hltVariants[] = {{OP_HLT, 1, 0}};

static const InstrVariant jmpVariants[] = {
	{OP_JMP_C, 2, 01},
	{OP_JMP_R, 2, 02},
	{OP_JMP_M, 2, 03},
	{OP_JMP_P, 2, 04},
	{OP_JMP_L, 2, 05},
};

static const InstrVariant jmpzVariants[] = {
	{OP_JMPZ_RC, 3, 021},
	{OP_JMPZ_RR, 3, 022},
	{OP_JMPZ_RM, 3, 023},
	{OP_JMPZ_RP, 3, 024},
	{OP_JMPZ_RL, 3, 025},
	{OP_JMPZ_MC, 3, 031},
	{OP_JMPZ_MR, 3, 032},
	{OP_JMPZ_MM, 3, 033},
	{OP_JMPZ_MP, 3, 034},
	{OP_JMPZ_ML, 3, 035},
	{OP_JMPZ_PC, 3, 041},
	{OP_JMPZ_PR, 3, 042},
	{OP_JMPZ_PM, 3, 043},
	{OP_JMPZ_PP, 3, 044},
	{OP_JMPZ_PL, 3, 045},
};

static const InstrVariant jmpnzVariants[] = {
	{OP_JMPNZ_RC, 3, 021},
	{OP_JMPNZ_RR, 3, 022},
	{OP_JMPNZ_RM, 3, 023},
	{OP_JMPNZ_RP, 3, 024},
	{OP_JMPNZ_RL, 3, 025},
	{OP_JMPNZ_MC, 3, 031},
	{OP_JMPNZ_MR, 3, 032},
	{OP_JMPNZ_MM, 3, 033},
	{OP_JMPNZ_MP, 3, 034},
	{OP_JMPNZ_ML, 3, 035},
	{OP_JMPNZ_PC, 3, 041},
	{OP_JMPNZ_PR, 3, 042},
	{OP_JMPNZ_PM, 3, 043},
	{OP_JMPNZ_PP, 3, 044},
	{OP_JMPNZ_PL, 3, 045},
};

#include "generated/generatedVariants.h"

#endif // KAL_VARIANTS_INC
