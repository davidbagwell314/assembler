g++ -o bin\assembler.exe src\assembler.cpp src\coff.cpp "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64\kernel32.lib" -I"C:\msys64\ucrt64\include" -I"include"
.\bin\assembler.exe

gcc -o test\main.exe test\main.obj "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64\kernel32.lib" & .\test\main.exe