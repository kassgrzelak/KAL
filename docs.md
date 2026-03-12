# KAL Docs

### Syntax
A KAL Program consists of a series statements, which are either labels or instructions. 
The convention is to have one statement per line, but KAL allows multiple.

### Instruction Statements

An instruction statement starts with an instruction mnemonic and is followed by the arguments for that instruction, 
separated by whitespace.
Here is an example:
```
mul %0 2
```

This multiplies the value stored in register 0 by 2 and stores the result in register 0.

And here is an example program that stores a constant in a register, multiplies it by 4, then outputs the result.
```
ld %0 3 ; Store the value of 3 in register 0.
mul %0 4 ; Multiply the value in register 0 by 4.
out %0 ; Output the value of register 0.
```

KAL is case insensitive.

### Labels

A label is a location in the code with an identifier so that it can be referred to later in order to jump to it.
A label is declared by writing its identifier followed by the label declaration operator (`:`). To use a label as an
operand to an instruction,  such as a jump instruction, you must prepend the label identifier with the label operand
operator (`.`).
```
start:
  out 0
jmp .start ; Output 0 infinitely.
```

Label identifiers must be exclusively alphanumeric, and you may not have more than 256 of them in a program.

### Comments

Comments are denoted with semicolons (`;`) and go until the end of the line. A comment may make up the entirety of a
line or appear at the end of a line.
```
; This is a line with a comment.
inc %4 ; Increment the value in register 4.
```

### Registers

There are eight 8-bit registers in the virtual CPU used by KAL. The values in a register are referred to by the register
operator (`%`) followed by the index of that register (0-7 inclusive). For example:
```
out %0 ; Output the value currently stored in register 0.
```

If you wish, you can also refer to the registers alphabetically. That is, `%a` refers to register 0, `%b` to 1, up to 
`%h` for 7.
```
out %a ; Also output the value in register 0.
```

### RAM

The 8-bit registers allow for the addressing of 256 bytes of RAM. Just like registers, you can access RAM using an index
from 0 to 255, inclusive, using the RAM operator (`$`).
```
out $0 ; Output the value currently stored in RAM location 0.
```

### RAM Pointers
To access the RAM location whose index is given by the value in a register (in other words, to dereference a pointer to
RAM), use the register dereference operator (`*`).
```
ld %0 3 ; Load register 0 with the value 3.
out *0 ; Output the value in RAM pointed to by register 0 (the value in RAM location 3).
```