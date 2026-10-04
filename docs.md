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
by default, but binary literals can be written by adding the prefix `0b`, octal by `0`, and hexadecimal by `0x` (all
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
You can use either the double quote (`"`) or single quote (`'`) character to denote a string constant, but you must 
start and end with the same character. The convention is to use single quotes for single character literals (e.g. `'c'`)
and double quotes for full strings, but this is not enforced.

You can use a backslash (`\`) to denote an ASCII escape code. This can be used to have hard-to-type characters in your
code using familiar names, such as `\n`. KAL only supports single-character escape codes. If there is no ASCII escape
code for the character following the backslash character, or KAL does not support that escape code, it will be 
equivalent to just typing that character without the backslash.

Note that unlike C/C++, the null character is not added for you at the end of string literals. If you want it, add `\0`
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

Label identifiers must be exclusively alphanumeric including underscores, and you may not have more than 256 of them in
a program. This is due to a fundamental limitation of the virtual 8-bit processor used by KAL. 

## The Preprocessor
The preprocessor runs before the program is compiled and allows you to define identifiers that will be replaced with 
other text. Preprocessor directives are indicated by a hashtag (`#`).

### #macro
The #macro directive defines a macro by giving its alphanumeric identifier followed by the text that should replace it, 
surrounded by backticks (`).
```
#macro EXAMPLE_MACRO `42`
out EXAMPLE_MACRO ; Outputs 42.
```

Macros are replaced wherever their name appears as an alphanumeric identifier. Macro names are case-insensitive, though 
it is convention to have macro identifiers in all-uppercase, and a macro may be used in the replacement text of another 
macro.
```
#macro EXAMPLE_MACRO `out 10`
#macro DOUBLED `EXAMPLE_MACRO EXAMPLE_MACRO`
DOUBLED ; Outputs 10 10.
```

The preprocessor repeats macro replacement until no more replacements can be made. It allows up to 64 preprocessing 
rounds.

### #unmacro
The #unmacro directive undefines a previously defined macro. Appearances of this macro will not be replaced after this 
directive.
```
#macro VALUE `10`
out VALUE ; Outputs 10.
#unmacro VALUE
#macro VALUE `20`
out VALUE ; Outputs 20.
```

After a macro is undefined, text with its name is left unchanged until it is defined again. It is an error to try to 
remove a macro that has not been defined.

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

#### Combination with Preprocessor Macros
You can combine these directives with preprocessor macros to create named arrays in your code.
```
#macro MY_ARRAY `0`
#datafrom $MY_ARRAY
1 2 3 4

mv %0 MY_ARRAY ; Register zero now holds the start of the array.
```