# KAL Instruction Set

## Instruction Signature Shorthand Key
C - Constant. \
M - Memory location: A register, RAM address, or RAM pointer. \
L - Label.

Where one argument type appears more than once in an instruction, they will be numbered for clarity (e.g., M0 and M1).

## Basic
### nop
Do nothing.
- `nop` - Do nothing.

### hlt
Stop execution of the program.
- `hlt` - Stop execution.

### mv
Move a value into a memory location.
- `mv M C` - Move C into M.
- `mv M1 M0` - Move the value at M0 into M1.

### out
Output a number to stdout.
- `out C` - Output a constant.
- `out M` - Output the value in a memory location.

### outa
Output an ASCII character to stdout.
- `out C` - Output a constant.
- `out M` - Output the value in a memory location.

## Arithmetic
### inc
Increment the value in a memory location.
- `inc M` - Increment the value in M. Identical to `add M 1`.

### dec
Decrement the value in a memory location.
- `dec M` - Decrement the value in M. Identical to `sub M 1`.

### add
Add to a value in a memory location.
- `add M C` - Add C to the value in M.
- `add M1 M0` - Add the value in M0 to the value in M1.

### sub
Subtract from a value in a memory location.
- `add M C` - Subtract C from the value in M.
- `add M1 M0` - Subtract the value in M0 from the value in M1.

### mul
Multiply a value in a memory location.
- `mul M C` - Multiply C by the value in M and store the result in M.
- `mul M1 M0` - Multiply the value in M0 by the value in M1 and store the result in 
M1.

### div
Divide a value in a memory location with truncation.
- `div M C` - Divide the value in M by C and store the result in M.
- `div M1 M0` - Divide the value in M1 by the value in M0 and store the result in
  M1.

## Bitwise Logic
### and
Perform bitwise AND on a value in a memory location.
- `and M C` - Perform bitwise AND with the value in M and C and store the result in M.
- `and M1 M0` - Perform bitwise AND with the value in M1 and the value in M0 and store the result in M1.

### or
Perform bitwise OR on a value in a memory location.
- `or M C` - Perform bitwise OR with the value in M and C and store the result in M.
- `or M1 M0` - Perform bitwise OR with the value in M1 and the value in M0 and store the result in M1.

### xor
Perform bitwise XOR on a value in a memory location.
- `xor M C` - Perform bitwise XOR with the value in M and C and store the result in M.
- `xor M1 M0` - Perform bitwise XOR with the value in M1 and the value in M0 and store the result in M1.

### not
Perform bitwise NOT on a value in a memory location.
- `not M C` - Perform bitwise NOT with the value in M and C and store the result in M.
- `not M1 M0` - Perform bitwise NOT with the value in M1 and the value in M0 and store the result in M1.

## Jumps
### jmp
Unconditional jump.
- `jmp C` - Jump a constant number of instructions forwards.
- `jmp M` - Jump M instructions forwards.
- `jmp L` - Jump to label L.

### jmpz
Conditional jump if zero.
- `jmpz M C` - Jump a constant number of instructions forwards only if the value in M is zero.
- `jmpz M1 M0` - Jump M0 instructions forwards only if the value in M1 is zero.
- `jmpz M L` - Jump to label L only if the value in M is zero.

### jmpnz
Conditional jump if not zero.
- `jmpnz M C` - Jump a constant number of instructions forwards only if the value in M is not zero.
- `jmpnz M1 M0` - Jump M0 instructions forwards only if the value in M1 is not zero.
- `jmpnz M L` - Jump to label L only if the value in M is not zero.

## Function calls
### call
Call a function.
- `call L` - Jump to label L and return to this code location when a ret instruction is executed.

### ret
Return from a function call.
- `ret` - Return from a function call. Executing this when the call stack is empty is a runtime error.