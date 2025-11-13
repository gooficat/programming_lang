section .bss

section .text
global _start

_start:
mov eax, 2 
push eax
mov eax, 4 
push eax
mov eax, 2 
pop ebx
imul eax, ebx
pop ebx
imul eax, ebx
push eax
mov eax, 2 
push eax
mov eax, 4 
push eax
mov eax, 2 
pop ebx
imul eax, ebx
pop ebx
add eax, ebx
pop ebx
add eax, ebx
ret
