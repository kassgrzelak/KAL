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
}

void freeVM(const VM* vm)
{
	freeBytecode(&vm->bytecode);
}

static void runtimeError(const VM* vm, const char* message)
{
	fprintf(stderr, "[byte 0x%04lx] Error: %s\n", vm->ip - vm->bytecode.code - 1, message);
}

#define CONST() *vm->ip++
#define REG() vm->registers[CONST()]
#define MEM() vm->ram[CONST()]
#define PTR() vm->ram[vm->registers[CONST()]]

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

static void INC_RHandler(VM* vm)
{
	++REG();
}

static void INC_MHandler(VM* vm)
{
	++MEM();
}

static void INC_PHandler(VM* vm)
{
	++PTR();
}

static void DEC_RHandler(VM* vm)
{
	--REG();
}

static void DEC_MHandler(VM* vm)
{
	--MEM();
}

static void DEC_PHandler(VM* vm)
{
	--PTR();
}

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

static void MV_RCHandler(VM* vm)
{
	uint8_t* reg = &REG();
	*reg = CONST();
}

static void MV_RRHandler(VM* vm)
{
	uint8_t* reg = &REG();
	*reg = REG();
}

static void MV_RMHandler(VM* vm)
{
	uint8_t* reg = &REG();
	*reg = MEM();
}

static void MV_RPHandler(VM* vm)
{
	uint8_t* reg = &REG();
	*reg = PTR();
}

static void MV_MCHandler(VM* vm)
{
	uint8_t* mem = &MEM();
	*mem = CONST();
}

static void MV_MRHandler(VM* vm)
{

	uint8_t* mem = &MEM();
	*mem = REG();
}

static void MV_MMHandler(VM* vm)
{

	uint8_t* mem = &MEM();
	*mem = MEM();
}

static void MV_MPHandler(VM* vm)
{

	uint8_t* mem = &MEM();
	*mem = PTR();
}

static void MV_PCHandler(VM* vm)
{
	uint8_t* ptr = &PTR();
	*ptr = CONST();
}

static void MV_PRHandler(VM* vm)
{

	uint8_t* ptr = &PTR();
	*ptr = REG();
}

static void MV_PMHandler(VM* vm)
{

	uint8_t* ptr = &PTR();
	*ptr = MEM();
}

static void MV_PPHandler(VM* vm)
{

	uint8_t* ptr = &PTR();
	*ptr = PTR();
}

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

	return INTERPRET_OK;
}

InterpretResult interpret(VM* vm, const char* source)
{
	initBytecode(&vm->bytecode);

	if (!compile(&vm->bytecode, vm->jumpTable, source))
		return INTERPRET_COMPILE_ERROR;

	vm->ip = vm->bytecode.code;
	return run(vm);
}
