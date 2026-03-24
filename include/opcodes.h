//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_OPCODES_INC
#define KAL_OPCODES_INC

#include "generated/generatedOpcodes.h"

#define OPCODES_X \
	X(NOP) \
	X(HLT) \
	X(JMP_L) \
	X(JMPZ_RL) \
	X(JMPZ_ML) \
	X(JMPZ_PL) \
	X(JMPNZ_RL) \
	X(JMPNZ_ML) \
	X(JMPNZ_PL) \
	GENERATED_OPCODES_X

#endif //KAL_OPCODES_INC