//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_OPCODES_INC
#define KAL_OPCODES_INC

#include "generated/generatedOpcodes.h"

#define OPCODES_X \
	X(NOP) \
	X(HLT) \
	X(JMP_C) \
	X(JMP_R) \
	X(JMP_M) \
	X(JMP_P) \
	X(JMP_L) \
	GENERATED_OPCODES_X

#endif //KAL_OPCODES_INC