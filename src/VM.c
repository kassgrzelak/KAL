//
// Created by kassie on 13/03/2026.
//

#include "VM.h"

#include "Compiler.h"

void initVM(VM* vm)
{

}

void freeVM(const VM* vm)
{
	freeBytecode(&vm->bytecode);
}

void interpret(VM* vm, const char* source)
{

}
