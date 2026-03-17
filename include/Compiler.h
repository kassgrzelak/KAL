//
// Created by kassie on 12/03/2026.
//

#ifndef KAL_COMPILER_H
#define KAL_COMPILER_H

#include "Bytecode.h"
#include "Scanner.h"

typedef enum
{
	// Note octal values. An AM value fits in three bits or one octal digit, and this is how instruction signatures are
	// written. E.g., 034 for 'instr M P'.
	AM_NONE  = 00,
	AM_CONST = 01,
	AM_REG   = 02,
	AM_MEM   = 03,
	AM_PTR   = 04,
	AM_LABEL = 05,
} AddressingMode;

AddressingMode tokenTypeToAddressingMode(TokenType type);
uint16_t signatureFromModes(const AddressingMode* modes, int count);

bool compile(Bytecode* bytecode, size_t* jumpTable, const char* source);

#endif //KAL_COMPILER_H