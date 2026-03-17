//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_VM_H
#define KAL_VM_H

#include "Compiler.h"

typedef struct
{
	Bytecode bytecode;
	uint8_t* ip;

	uint8_t registers[8];
	uint8_t ram[256];

	size_t jumpTable[256];
} VM;

typedef enum
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM(VM* vm);
void freeVM(const VM* vm);

InterpretResult interpret(VM* vm, const char* source);

#endif //KAL_VM_H