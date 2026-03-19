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

#include "generated/generatedVariants.h"

#endif // KAL_VARIANTS_INC
