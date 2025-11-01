@echo off

set "KERNEL32=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64\kernel32.lib"

make

.\build\assembler.exe

gcc -o test\main.exe test\main.obj "%KERNEL32%" & .\test\main.exe