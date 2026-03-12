//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_BYTECODE_H
#define KAL_BYTECODE_H

#include "common.h"

typedef struct
{
	size_t capacity;
	size_t count;
	uint8_t* code;
} Bytecode;

void initBytecode(Bytecode* code);
void freeBytecode(Bytecode* code);

#endif //KAL_BYTECODE_H