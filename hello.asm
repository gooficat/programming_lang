section .text
global _start

_start:
	mov eax, 82 
push eax
mov eax, 64 
pop ebx
add eax, ebx
ret
