# KAL Docs

## Syntax
A KAL Program consists of a series of statements, which are either labels or instructions. 
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
mv %0 3 ; Store the value of 3 in register 0.
mul %0 4 ; Multiply the value in register 0 by 4.
out %0 ; Output the value of register 0.
```

KAL is completely case-insensitive, so you may write instruction mnemonics (or indeed any text) in all-lowercase (as in
these docs), in all-uppercase, or any other capitalization style you like.

### Comments
Comments are denoted with semicolons (`;`) and go until the end of the line. A comment may make up the entirety of a
line or appear at the end of a line.
```
; This is a line with a comment.
inc %4 ; Increment the value in register 4.
```

Inline comments may also be formed by surrounding commented text in semicolons (`;`). These comments may not span lines.

Allowed:
```
mv ;pos; %0 ;newPos; *1
```
Not allowed:
```
inc %0 ; Increment
register zero. ; mv $0 %0
```
`register zero. ` will be read as code and `mv $0 %0` will be read as a comment.

## Operand Types

### Constants
Constants are written as just plain numbers, with no special operator denoting them. They are parsed as decimal numbers
by default, but binary literals can be written by adding the prefix `0b`, octal by `0`, and hexadecimal by `0x` (All
case-insensitive). Letters A through F in hexadecimal number literals are also case-insensitive.
```
out 0b1001 ; Binary.
out 0123   ; Octal.
out 123    ; Decimal.
out 0xcf   ; Hexadecimal.
```

#### Strings
KAL supports string constants. These are exactly equivalent to just writing out a list of the ASCII values for each 
character.
```
"hello" ; Is exactly the same as just putting:
104 101 108 108 111 ; in your code.
```
You can use either the double quote (`"`) or single quote (`'`) character to denote a string constant. The convention is
to use single quotes for single character literals (e.g. `'c'`) and double quotes for full strings, but this is not
enforced.

You can use a backslash (`\`) to denote an ASCII escape code. This can be used to have hard-to-type characters in your
code using familiar names, such as `\n`. KAL only supports single-character escape codes. If there is no ASCII escape
code for the character following the backslash character, or KAL does not support that escape code, it will be 
equivalent to just typing that character without the backslash.

Note that, unlike C/C++, the null character is not added for you at the end of string literals. If you want it, add `\0`
to the end of your string.

### Registers
There are eight 8-bit registers in the virtual CPU used by KAL. The values in a register are referred to by the register
operator (`%`) followed by the index of that register (0–7 inclusive). For example:
```
out %0 ; Output the value currently stored in register 0.
```

If you wish, you can also refer to the registers alphabetically. That is, `%a` refers to register 0, `%b` to 1, up to
`%h` for 7.
```
out %a ; Also output the value in register 0.
```
Note that `%a` is different from `%0xa`. The first refers to register zero and the second to (the non-existent)
register ten.

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
mv %0 3 ; Load register 0 with the value 3.
out *0 ; Output the value in RAM pointed to by register 0 (the value in RAM location 3).
```

### Labels
A label names a location in the code so that it can be jumped to later by name.

A label is declared by writing its identifier followed by the label declaration operator (`:`). To use a label as an
operand to an instruction, such as a jump instruction, you must prepend the label identifier with the label operand
operator (`.`).
```
start:
  out 0
jmp .start ; Output 0 infinitely.
```

Label identifiers must be exclusively alphanumeric, and you may not have more than 256 of them in a program.

## Assembler Directives
Assembler directives are indicated by a hashtag (`#`) and do not translate to any instructions in the compiled code,
but rather perform other tasks.

### #datafrom and #datato
These directives both preload ram with an array of data, but in slightly different ways.

The #datafrom directive indicates that the following bytes should be present in a contiguous range in RAM starting from
the address specified. For example:
```
#datafrom $10
1 2 3 4
```
RAM address 10 will contain the number 1, address 11 will contain the number 2, and so on.
```
RAM Address:   | 10 | 11 | 12 | 13 |
Stored Number: |  1 |  2 |  3 |  4 |
```

There is also the #datato directive, which works the same way except that the elements you give it are placed *up to and
including* the given address. For example:
```
#datato $255
1 2 3 4
```
So, RAM address 255 will contain the number 4, address 254 will contain 3, and so on.
```
RAM Address:   | 252 | 253 | 254 | 255 |
Stored Number: |   1 |   2 |   3 |   4 |
```

### #memalias 
You can use the #memalias directive to name any RAM address with a memorable string identifier, which can then be used
anywhere a RAM address is expected. Here is an example:
```
#memalias $my_number $0

mv $my_number 10
out $my_number
```
This program creates an alias for RAM address 0 called `my_number`, uses it to move the value of 10 into it, then
prints it to the screen. Note that you still must prepend the name with the RAM operator (`$`).

Because the #memalias directive is dealt with in a separate pass before any instruction statements are compiled, a RAM
alias doesn't even have to be defined above where it's used in the program, making this a completely valid KAL program:
```
mv $my_number 10
out $my_number

#memalias $my_number $0
```

#### Aliased RAM Address Operator
However, it would be nice to be able to recover the actual address the identifier is aliased to while still using that 
handy identifier. This is what the aliased RAM address operator (`&`) is for.

When followed by an aliased RAM address identifier, it evaluates to the numerical address that identifier is aliased to.
```
#memalias $named_location $10
mv %0 &named_location ; Register 0 will now contain the number 10.
```
Note how the second line is different to:
```
mv %0 $named_location
```
As this instruction would move whatever value is in RAM address 10 into register 0.

You can combine this with the #memalias and #datafrom directives to easily create named arrays in your code.
```
#memalias $my_text $100
#datafrom &my_text
"hello!\0"
```

Note that the result of this operator is a constant value and will be treated as such in the operands to any instruction
or assembler directive.