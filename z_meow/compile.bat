@echo OFF

..\build\meowlang_cpp %1.meow %1.asm
nasm -fwin64 %1.asm -o %1.obj
link.exe /subsystem:console /entry:start %1.obj kernel32.lib /OUT:%1.exe