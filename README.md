# BF-Compiler
Compiler for Brainfuck https://en.wikipedia.org/wiki/Brainfuck

Does basic optimizations. Such as removing redundant instructions and combining multiple instructions into one.

Overflow/Underflow are undefined.
Programs that rely on overflow/underflow behavior can only be ran in interpreted mode.

">"	increment the data pointer

"<"	decrement the data pointer

"+"	increment the byte at the data pointer.

"-"	decrement the byte at the data pointer.

"."	output the byte at the data pointer.

","	accept one byte of input, storing its value in the byte at the data pointer.

"["	if the byte at the data pointer is zero, then instead of moving the instruction pointer forward to the next command, jump it forward to the command after the matching ] command.

"]"	if the byte at the data pointer is nonzero, then instead of moving the instruction pointer forward to the next command, jump it back to the command after the matching [ command.
