//
// Created by kassie on 17/03/2026.
//
#ifndef KAL_VARIANTS_INC
#define KAL_VARIANTS_INC

static const InstrVariant nopVariants[] = {{OP_NOP, 0}};

static const InstrVariant hltVariants[] = {{OP_HLT, 0}};

static const InstrVariant callVariants[] = {{OP_CALL_L, 05}};

static const InstrVariant retVariants[] = {{OP_RET, 0}};

static const InstrVariant jmpVariants[] = {{OP_JMP_L, 05}};

static const InstrVariant jmpzVariants[] = {
	{OP_JMPZ_RL, 025},
	{OP_JMPZ_ML, 035},
	{OP_JMPZ_PL, 045},
};

static const InstrVariant jmpnzVariants[] = {
	{OP_JMPNZ_RL, 025},
	{OP_JMPNZ_ML, 035},
	{OP_JMPNZ_PL, 045},
};

#include "generated/generatedVariants.h"

#endif // KAL_VARIANTS_INC
