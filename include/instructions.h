//
// Created by kassie on 16/03/2026.
//
#ifndef KAL_INSTRUCTIONS_H
#define KAL_INSTRUCTIONS_H

#include "Scanner.h"
#include "opcodes.h"
#include "generated/generatedTableEntries.h"
#include "generated/generatedOpcodeLengths.h"

// Steps to add a new instruction:
// If the instructions is repetitive enough, you can add its template to dev-tools/boilerplateGenerator.py and have it
// generate all the code for you, no other work needed.
// Otherwise, you'll have to:
// 1. Add the token to mnemonicTokens.h.
// 2. Add every opcode for the instruction to opcodes.h.
// 3. Add every variant to variants.h.
// 4. Add an instruction table entry to instructions.h.
// 5. Add all opcode lengths to the opcode lengths table in instructions.h.
// 6. Add handlers to vmHandlers.h.

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

#include "variants.h"

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
	GENERATED_TABLE_ENTRIES
};

static const uint8_t opcodeLengthTable[] = {
	[OP_NOP] = 1,
	[OP_HLT] = 1,
	[OP_JMP_L] = 2,
	[OP_JMPZ_RL] = 3,
	[OP_JMPZ_ML] = 3,
	[OP_JMPZ_PL] = 3,
	[OP_JMPNZ_RL] = 3,
	[OP_JMPNZ_ML] = 3,
	[OP_JMPNZ_PL] = 3,
	GENERATED_OPCODE_LENGTHS
};

#undef TABLE_ENTRY

static const uint8_t INSTR_COUNT =  sizeof(instrTable) / sizeof(InstrDef);

#endif // KAL_INSTRUCTIONS_H
