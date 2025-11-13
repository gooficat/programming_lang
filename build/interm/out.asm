section .bss
x resb 4

section .text
global _start

_start:
mov eax, 3 
mov [x], eax
mov eax, [x]
push eax
mov eax, [x]
pop ebx
imul eax, ebx
push eax
mov eax, [x]
pop ebx
add eax, ebx
ret
