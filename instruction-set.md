# KAL Instruction Set

## Instruction Signature Shorthand Key
C - Constant. \
M - Memory location: A register, RAM address, or RAM pointer. \
L - Label.

Where one argument type appears more than once in an instruction, they may be numbered for clarity (e.g., C0 and C1).

## Basic
### nop
Do nothing.
- `nop` - Do nothing.

### hlt
Stop execution of the program.
- `hlt` - Stop execution.

### mv
Move a value into a memory location.
- `mv M C` - Move constant C into memory location M.
- `mv M1 M0` - Move the value at memory location M0 into memory location M1.

### out
Output a value to stdout.
- `out C` - Output a constant.
- `out M` - Output the value in a memory location.

## Arithmetic
### inc
Increment the value in a memory location.
- `inc M` - Increment the value in memory location M. Identical to `add M 1`.

### dec
Decrement the value in a memory location.
- `dec M` - Decrement the value in memory location M. Identical to `sub M 1`.

### add
Add to a value in a memory location.
- `add M C` - Add C to the value in memory location M.
- `add M1 M0` - Add the value in memory location M0 to the value in memory location M1.

### sub
Subtract from a value in a memory location.
- `add M C` - Subtract C from the value in memory location M.
- `add M1 M0` - Subtract the value in memory location M0 from the value in memory location M1.

## Jumps
### jmp
Unconditional jump.
- `jmp L` - Jump to label L.