//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_VM_HANDLERS_INC
#define KAL_VM_HANDLERS_INC

static void NOPHandler(VM* vm) { }

static void HLTHandler(VM* vm) { }

static void JMP_LHandler(VM* vm)
{
	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPZ_RLHandler(VM* vm)
{
	if (REG() != 0)
	{
		vm->ip += 2;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPZ_MLHandler(VM* vm)
{
	if (MEM() != 0)
	{
		vm->ip += 2;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPZ_PLHandler(VM* vm)
{
	if (PTR() != 0)
	{
		vm->ip += 2;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPNZ_RLHandler(VM* vm)
{
	if (REG() == 0)
	{
		vm->ip += 2;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPNZ_MLHandler(VM* vm)
{
	if (MEM() == 0)
	{
		vm->ip += 2;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPNZ_PLHandler(VM* vm)
{
	if (PTR() == 0)
	{
		vm->ip += 2;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

#include "generated/generatedHandlers.h"

#endif //KAL_VM_HANDLERS_INC