//
// Created by kassie on 17/03/2026.
//

#ifndef KAL_VM_HANDLERS_INC
#define KAL_VM_HANDLERS_INC

static void NOPHandler(VM* vm) { }

static void HLTHandler(VM* vm) { }

static void OUT_CHandler(VM* vm)
{
	printf("%d\n", CONST());
}

static void OUT_RHandler(VM* vm)
{
	printf("%d\n", REG());
}

static void OUT_MHandler(VM* vm)
{
	printf("%d\n", MEM());
}

static void OUT_PHandler(VM* vm)
{
	printf("%d\n", PTR());
}

#include "generated/incHandlers.inc"
#include "generated/decHandlers.inc"

static void JMP_CHandler(VM* vm)
{
	runtimeError(vm, "Not implemented.");
}

static void JMP_RHandler(VM* vm)
{
	runtimeError(vm, "Not implemented.");
}

static void JMP_MHandler(VM* vm)
{
	runtimeError(vm, "Not implemented.");
}

static void JMP_PHandler(VM* vm)
{
	runtimeError(vm, "Not implemented.");
}

static void JMP_LHandler(VM* vm)
{
	vm->ip = &vm->bytecode.code[vm->jumpTable[CONST()]];
}

#include "generated/mvHandlers.inc"
#include "generated/addHandlers.inc"
#include "generated/subHandlers.inc"

#endif //KAL_VM_HANDLERS_INC