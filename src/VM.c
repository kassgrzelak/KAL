//
// Created by kassie on 13/03/2026.
//

#include "VM.h"

#include "Compiler.h"

void initVM(VM* vm)
{
	initBytecode(&vm->bytecode);
}

void freeVM(const VM* vm)
{
	freeBytecode(&vm->bytecode);
}

static InterpretResult run(VM* vm)
{

	return INTERPRET_OK;
}

InterpretResult interpret(VM* vm, const char* source)
{
	initBytecode(&vm->bytecode);

	if (!compile(&vm->bytecode, vm->jumpTable, source))
		return INTERPRET_COMPILE_ERROR;

	return run(vm);
}
