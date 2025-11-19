DEFAULT REL
section .bss
msg resb 6
section .data
section .text
extern ExitProcess
global start

start:
    sub rsp, 40
    call entry
    add rsp, 40
    call ExitProcess
extern GetModuleHandleA
extern MessageBoxA
hello:
mov byte [ msg + 0 ] , 72 
mov byte [ msg + 1 ] , 101 
mov byte [ msg + 2 ] , 108 
mov byte [ msg + 3 ] , 108 
mov byte [ msg + 4 ] , 111 
mov byte [ msg + 5 ] , 0 
mov rcx , 0 
lea rdx , [ msg ] 
lea r8 , [ msg ] 
mov r9 , 0 
call MessageBoxA 
ret 


entry:
call hello
mov rcx , 0 

