//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_VM_HANDLERS_INC
#define KAL_VM_HANDLERS_INC

static void NOPHandler(VM* vm) { }

static void HLTHandler(VM* vm) { }

static void CALL_LHandler(VM* vm)
{
	if (vm->callDepth >= 8)
	{
		runtimeError(vm, "Exceeded max call depth of 8.");
		return;
	}

	vm->callStack[vm->callDepth++] = vm->ip + 1;
	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void RETHandler(VM* vm)
{
	if (vm->callDepth == 0)
	{
		runtimeError(vm, "No call to return from.");
		return;
	}

	vm->ip = vm->callStack[--vm->callDepth];
}

static void JMP_LHandler(VM* vm)
{
	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPZ_RLHandler(VM* vm)
{
	if (REG() != 0)
	{
		vm->ip += 1;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPZ_MLHandler(VM* vm)
{
	if (MEM() != 0)
	{
		vm->ip += 1;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPZ_PLHandler(VM* vm)
{
	if (PTR() != 0)
	{
		vm->ip += 1;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPNZ_RLHandler(VM* vm)
{
	if (REG() == 0)
	{
		vm->ip += 1;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPNZ_MLHandler(VM* vm)
{
	if (MEM() == 0)
	{
		vm->ip += 1;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

static void JMPNZ_PLHandler(VM* vm)
{
	if (PTR() == 0)
	{
		vm->ip += 1;
		return;
	}

	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

#include "generated/generatedHandlers.h"

#endif //KAL_VM_HANDLERS_INC