# README.md

Currently only assembles to Windows COFF object files. I will try to add support for ELF (I wrote a simple ELF executable previously, however it was done manually so I didn't write an assembler for it)

Currently only assembles x86_64 machine code (once I can find a way to store all opcodes for each instruction because there are multiple opcodes and various bits of data for each instruction)

## Resources

https://learn.microsoft.com/en-us/windows/win32/debug/pe-format  
https://wiki.osdev.org/X86-64_Instruction_Encoding  
https://wiki.osdev.org/CPU_Registers_x86-64  
https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html  
