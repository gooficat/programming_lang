section .bss
section .data
section .text
extern ExitProcess
global start
escape:
mov rcx , [ rsp + 8 ] 
call ExitProcess 

start:
push 2
call escape

