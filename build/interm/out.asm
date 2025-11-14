section .bss
x resb 4
v resb 4
section .data

section .text
global _start

_start:
mov eax, 3 
mov [x], eax
mov eax, 2 
mov [v], eax


xor eax , eax 
mov eax , [ x ] 
add eax , 4 
mov [ x ] , eax 
mov eax, [x]
push eax
mov eax, [x]
pop ebx
add eax, ebx
ret
