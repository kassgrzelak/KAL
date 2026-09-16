//
// Created by kassie on 13/03/2026.
//

#include "VM.h"

#include <stdio.h>

#include "Compiler.h"
#include "instructions.h"

void initVM(VM* vm)
{
	initBytecode(&vm->bytecode);
	vm->callDepth = 0;
}

void freeVM(const VM* vm)
{
	freeBytecode(&vm->bytecode);
}

static void runtimeError(const VM* vm, const char* message)
{
	printf("[byte 0x%04llx] Error: %s\n", vm->ip - vm->bytecode.code - 1, message);
}

#define CONST() *vm->ip++
#define REG() vm->registers[CONST()]
#define MEM() vm->ram[CONST()]
#define PTR() vm->ram[vm->registers[CONST()]]

#include "vmHandlers.h"

typedef void (*VMHandler)(VM*);

static VMHandler vmHandlerTable[OPCODE_COUNT] = {
#define X(opcode) \
	[OP_##opcode] = &opcode##Handler,
	OPCODES_X
#undef X
};

static InterpretResult run(VM* vm)
{
	for (uint8_t opcode = *vm->ip++; opcode != OP_HLT; opcode = *vm->ip++)
	{
		if (opcode >= OPCODE_COUNT)
		{
			runtimeError(vm, "Illegal instruction.");
			return INTERPRET_RUNTIME_ERROR;
		}

		vmHandlerTable[opcode](vm);
	}

	// Reset call depth after each run in case it didn't end on 0.
	vm->callDepth = 0;

	return INTERPRET_OK;
}

InterpretResult interpret(VM* vm, const char* source)
{
	initBytecode(&vm->bytecode);

	if (!compile(&vm->bytecode, vm->jumpTable, vm->ram, source))
		return INTERPRET_COMPILE_ERROR;

	vm->ip = vm->bytecode.code;
	return run(vm);
}
