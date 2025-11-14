section .bss
x resb 4
section .data

section .text
global _start

_start:
mov eax, 3 
mov [x], eax


xor eax , eax 
mov eax , [ x ] 
add eax , 4 
mov [ x ] , eax 
mov eax, [x]
ret
