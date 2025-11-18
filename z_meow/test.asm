DEFAULT REL
section .bss
i resb 8
section .data
section .text
extern ExitProcess
global start
extern MessageBoxA 
escape:
mov rcx , [ rsp + 8 ] 
call ExitProcess 


start:
mov  [i],  2
push [i]
push 8
call escape
pop r15
pop r15

