//
// Created by kassie on 11/03/2026.
//

#ifndef KAL_VM_H
#define KAL_VM_H
#include "common.h"

typedef struct
{
	size_t jumpTable[256];
	Bytecode code;
} VM;

#endif //KAL_VM_H