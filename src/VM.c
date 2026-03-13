//
// Created by kassie on 13/03/2026.
//

#include "VM.h"

void initVM(VM* vm)
{

}

void freeVM(const VM* vm)
{
	freeBytecode(&vm->bytecode);
}
