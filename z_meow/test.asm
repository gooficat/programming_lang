section .bss
section .data
section .text
extern ExitProcess
global start
start:
mov rcx, 257
call ExitProcess
