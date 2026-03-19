//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_VM_HANDLERS_INC
#define KAL_VM_HANDLERS_INC

static void NOPHandler(VM* vm) { }

static void HLTHandler(VM* vm) { }

static void JMP_CHandler(VM* vm)
{
	for (uint8_t skipNum = CONST(); skipNum > 0; --skipNum)
		vm->ip += opcodeLengthTable[*vm->ip];
}

static void JMP_RHandler(VM* vm)
{
	for (uint8_t skipNum = REG(); skipNum > 0; --skipNum)
		vm->ip += opcodeLengthTable[*vm->ip];
}

static void JMP_MHandler(VM* vm)
{
	for (uint8_t skipNum = MEM(); skipNum > 0; --skipNum)
		vm->ip += opcodeLengthTable[*vm->ip];
}

static void JMP_PHandler(VM* vm)
{
	for (uint8_t skipNum = PTR(); skipNum > 0; --skipNum)
		vm->ip += opcodeLengthTable[*vm->ip];
}

static void JMP_LHandler(VM* vm)
{
	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

#include "generated/generatedHandlers.h"

#endif //KAL_VM_HANDLERS_INC