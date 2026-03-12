//
// Created by kassie on 12/03/2026.
//

#ifndef KAL_BYTECODE_H
#define KAL_BYTECODE_H

#include "common.h"

typedef struct
{
	uint8_t* code;
	size_t capacity;
	size_t count;
} Bytecode;

void initBytecode(Bytecode* bytecode);
void writeBytecode(Bytecode* bytecode, uint8_t byte);
void freeBytecode(Bytecode* bytecode);

#endif //KAL_BYTECODE_H