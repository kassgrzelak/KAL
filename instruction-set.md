# KAL Instruction Set

## Instruction Signature Shorthand Key
C - Constant. \
M - Memory location: A register, RAM address, or RAM pointer. \
L - Label.

Where one argument type appears more than once in an instruction, they may be numbered for clarity (e.g., C0 and C1).

## Basic
### nop
Do nothing.
- nop - Do nothing.

### ld
Load a value into a memory location.
- ld M C - Load constant C into memory location M.
- ld M1 M0 - Load the value at memory location M0 into memory location M1.

### out
Output a value to stdout.
- out C - Output a constant.
- out M - Output the value in a memory location.

## Jumps
### jmp
Unconditional jump.
- jmp C - jump a constant number of **instructions** ahead.
- jmp M - jump `n` instructions ahead, where n is the value held in memory location M.
- jmp L - Jump to label L.