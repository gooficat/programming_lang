@echo OFF
if not exist "build" (
    mkdir build
)
cd build
if not exist "interm" (
    mkdir interm
)
cd ..
py meow.py %1 build/interm/out.asm

nasm -fwin32 build/interm/out.asm -o build/interm/out.obj
link.exe /subsystem:console /entry:start build/interm/out.obj kernel32.lib /OUT:build/out.exe