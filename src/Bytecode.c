//
// Created by kassie on 12/03/2026.
//
#include "Bytecode.h"

#include "memory.h"

void initBytecode(Bytecode* bytecode)
{
	INIT_ARRAY(bytecode, code);
}

void writeBytecode(Bytecode* bytecode, const uint8_t byte)
{
	WRITE_ARRAY(uint8_t, bytecode, code, byte);
}

void freeBytecode(const Bytecode* bytecode)
{
	free(bytecode->code);
}
