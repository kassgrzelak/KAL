//
// Created by kassie on 16/03/2026.
//
#ifndef KAL_INSTRUCTIONS_H
#define KAL_INSTRUCTIONS_H

#include "Scanner.h"
#include "opcodes.inc"

// Checklist for adding a new instruction:
// 1. Add the token to mnemonicTokens.inc.
// 2. Add every opcode for the instruction to opcodes.inc (or generate them with boilerplateGenerator.py).
// 3. Add every variant to variants.inc (or generate them with boilerplateGenerator.py).
// 4. Add a table entry to instructions.h.
// 5. Add handlers to VM.c (or generate them with boilerplateGenerator.py).

typedef enum
{
#define X(opcode) \
	OP_##opcode,
	OPCODES_X
#undef X

	OPCODE_COUNT
} Opcode;

typedef struct
{
	uint8_t opcode;
	uint8_t length;
	uint16_t signature;
} InstrVariant;

#include "variants.inc"

typedef struct
{
	TokenType token;
	const char* mnemonic;
	int operandCount;
	const InstrVariant* variants;
	int variantCount;
} InstrDef;

#define TABLE_ENTRY(nameCaps, nameLower, operandCount) \
	{TOKEN_##nameCaps, #nameLower, operandCount, nameLower##Variants, sizeof(nameLower##Variants) / sizeof(InstrVariant)},

static const InstrDef instrTable[] = {
	TABLE_ENTRY(NOP, nop, 0)
	TABLE_ENTRY(HLT, hlt, 0)
	TABLE_ENTRY(OUT, out, 1)
	TABLE_ENTRY(INC, inc, 1)
	TABLE_ENTRY(DEC, dec, 1)
	TABLE_ENTRY(JMP, jmp, 1)
	TABLE_ENTRY(MV, mv, 2)
	TABLE_ENTRY(ADD, add, 2)
	TABLE_ENTRY(SUB, sub, 2)
};

#undef TABLE_ENTRY

static const uint8_t INSTR_COUNT =  sizeof(instrTable) / sizeof(InstrDef);

#endif // KAL_INSTRUCTIONS_H
