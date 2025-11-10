section .bss
i resb 4

section .text
global _start

_start:
	mov eax, 32 
mov [i], eax
	mov eax, 82 
push eax
mov eax, 64 
push eax
mov eax, [i]
pop ebx
add eax, ebx
pop ebx
add eax, ebx
ret
