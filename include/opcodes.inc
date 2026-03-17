//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_OPCODES_INC
#define KAL_OPCODES_INC

#define OPCODES_X \
	X(NOP) \
	X(HLT) \
	X(OUT_C) \
	X(OUT_R) \
	X(OUT_M) \
	X(OUT_P) \
	X(INC_R) \
	X(INC_M) \
	X(INC_P) \
	X(DEC_R) \
	X(DEC_M) \
	X(DEC_P) \
	X(JMP_C) \
	X(JMP_R) \
	X(JMP_M) \
	X(JMP_P) \
	X(JMP_L) \
	X(MV_RC) \
	X(MV_RR) \
	X(MV_RM) \
	X(MV_RP) \
	X(MV_MC) \
	X(MV_MR) \
	X(MV_MM) \
	X(MV_MP) \
	X(MV_PC) \
	X(MV_PR) \
	X(MV_PM) \
	X(MV_PP) \
	X(ADD_RC) \
	X(ADD_RR) \
	X(ADD_RM) \
	X(ADD_RP) \
	X(ADD_MC) \
	X(ADD_MR) \
	X(ADD_MM) \
	X(ADD_MP) \
	X(ADD_PC) \
	X(ADD_PR) \
	X(ADD_PM) \
	X(ADD_PP) \
	X(SUB_RC) \
	X(SUB_RR) \
	X(SUB_RM) \
	X(SUB_RP) \
	X(SUB_MC) \
	X(SUB_MR) \
	X(SUB_MM) \
	X(SUB_MP) \
	X(SUB_PC) \
	X(SUB_PR) \
	X(SUB_PM) \
	X(SUB_PP) \

#endif //KAL_OPCODES_INC