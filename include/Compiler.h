//
// Created by kassie on 12/03/2026.
//

#ifndef KAL_COMPILER_H
#define KAL_COMPILER_H

#include "Bytecode.h"
#include "Scanner.h"

// IMPORTANT: Checklist for adding a new instruction:
// - Add token to TokenType enum in Scanner.h.
// - Add scanning support in mnemonic() in Scanner.c.
// - Add debug printing support in printToken() in Scanner.c.
// - Add opcodes for all addressing modes to appropriate opcode enum in Compiler.h.
// - Add offset to appropriate getOffset() function in Compiler.c.
// - Add parser to Compiler.c to handle all addressing modes.
// - Add parser to mnemonicParserTable in Compiler.c.
// - Add execution support to VM.c.

typedef enum
{
	// Note octal values. An AM value fits in three bits or one octal digit, and this is how instruction signatures will
	// be written. E.g., 034 for 'instr M P'.
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