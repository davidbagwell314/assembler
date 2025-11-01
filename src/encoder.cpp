#include <encoder.h>

#include <iostream>
#include <iomanip>

/*

Much of the following comments come from the following sources:
    https://wiki.osdev.org/X86-64_Instruction_Encoding#ModR/M_and_SIB_bytes
    Intel® 64 and IA-32 Architectures Software Developer's Manual Combined Volumes 2A, 2B, 2C, and 2D: Instruction Set Reference, A- Z




Registers:

X.Reg	8-bit GP	16-bit GP	32-bit GP	64-bit GP	80-bit x87	64-bit MMX	128-bit XMM	256-bit YMM	16-bit Segment	32-bit Control	32-bit Debug
0.000	AL	        AX	        EAX	        RAX	        ST0	        MMX0	    XMM0	    YMM0	    ES	            CR0	            DR0
0.001	CL	        CX	        ECX	        RCX	        ST1	        MMX1	    XMM1	    YMM1	    CS	            CR1	            DR1
0.010	DL	        DX	        EDX	        RDX	        ST2	        MMX2	    XMM2	    YMM2	    SS	            CR2	            DR2
0.011	BL	        BX	        EBX	        RBX	        ST3	        MMX3	    XMM3	    YMM3	    DS	            CR3	            DR3
0.100	AH, SPL 	SP	        ESP	        RSP	        ST4	        MMX4	    XMM4	    YMM4	    FS	            CR4	            DR4
0.101	CH, BPL 	BP	        EBP	        RBP	        ST5	        MMX5	    XMM5	    YMM5	    GS	            CR5	            DR5
0.110	DH, SIL 	SI	        ESI	        RSI	        ST6	        MMX6	    XMM6	    YMM6	    -	            CR6	            DR6
0.111	BH, DIL 	DI	        EDI	        RDI	        ST7	        MMX7	    XMM7	    YMM7	    -	            CR7	            DR7
1.000	R8L	        R8W	        R8D	        R8	        -	        MMX0	    XMM8	    YMM8	    ES	            CR8	            DR8
1.001	R9L	        R9W	        R9D	        R9	        -	        MMX1	    XMM9	    YMM9	    CS	            CR9	            DR9
1.010	R10L	    R10W	    R10D	    R10	        -	        MMX2	    XMM10   	YMM10	    SS	            CR10	        DR10
1.011	R11L	    R11W	    R11D	    R11	        -	        MMX3	    XMM11   	YMM11	    DS	            CR11	        DR11
1.100	R12L	    R12W	    R12D	    R12	        -	        MMX4	    XMM12   	YMM12	    FS	            CR12	        DR12
1.101	R13L	    R13W	    R13D	    R13	        -	        MMX5	    XMM13   	YMM13	    GS	            CR13	        DR13
1.110	R14L	    R14W	    R14D	    R14	        -	        MMX6	    XMM14   	YMM14	    -	            CR14	        DR14
1.111	R15L	    R15W	    R15D	    R15	        -	        MMX7	    XMM15   	YMM15	    -	            CR15	        DR15

When any REX prefix is used, SPL, BPL, SIL and DIL are used. Otherwise, AH, CH, DH and BH are used.




REX Prefix:

  7                           0
+---+---+---+---+---+---+---+---+
| 0   1   0   0 | W | R | X | B |
+---+---+---+---+---+---+---+---+

REX.W: 1 = 64 Bit Operand Size
REX.R: Extension of the ModR/M reg field
REX.X: Extension of the SIB index field
REX.B: Extension of the ModR/M r/m field, SIB base field, or Opcode reg field


ModR/M Byte:

  7                           0
+---+---+---+---+---+---+---+---+
|  mod  |    reg    |     rm    |
+---+---+---+---+---+---+---+---+

MODRM.mod: When 0b11, register-direct addressing mode is used; otherwise register-indirect addressing mode is used.
MODRM.reg: Either 3-bit opcode extension or 3-bit register reference. Extended to 4 bits by REX.R, VEX.~R or XOP.~R
MODRM.rm: Specifies a direct or indirect register operand, optionally with a displacement. Extended to 4 bits by REX.B, VEX.~B or XOP.~B


SIB Byte:

  7                           0
+---+---+---+---+---+---+---+---+
| scale |   index   |    base   |
+---+---+---+---+---+---+---+---+

SIB.scale: Indicates scaling factor of SIB.index, where s (as used in the tables) equals 2^SIB.scale.
SIB.index: Which index register to use. Extended to 4 bits by REX.X, VEX.~X or XOP.~X
SIB.base: Which base register to use. Extended to 4 bits by REX.B, VEX.~B or XOP.~B




Opcode encoding entry symbols:

• NP — Indicates the use of 66/F2/F3 prefixes (beyond those already part of the instructions opcode) are not
allowed with the instruction. Such use will either cause an invalid-opcode exception (#UD) or result in the
encoding for a different instruction.

• NFx — Indicates the use of F2/F3 prefixes (beyond those already part of the instructions opcode) are not
allowed with the instruction. Such use will either cause an invalid-opcode exception (#UD) or result in the
encoding for a different instruction.

• REX.W — Indicates the use of a REX prefix that affects operand size or instruction semantics. Note that REX
prefixes that promote legacy instructions to 64-bit behavior are not listed explicitly in the opcode column.

• /digit — A digit between 0 and 7 indicates that the ModR/M byte of the instruction uses only the r/m (register
or memory) operand. The reg field contains the digit that provides an extension to the instruction's opcode.

• /r — Indicates that the ModR/M byte of the instruction contains a register operand and an r/m operand.

• cb, cw, cd, cp, co, ct — A 1-byte (cb), 2-byte (cw), 4-byte (cd), 6-byte (cp), 8-byte (co) or 10-byte (ct) value
following the opcode. This value is used to specify a code offset and possibly a new value for the code segment
register.

• ib, iw, id, io — A 1-byte (ib), 2-byte (iw), 4-byte (id) or 8-byte (io) immediate operand to the instruction that
follows the opcode, ModR/M bytes or scale-indexing bytes. The opcode determines if the operand is a signed
value. All words, doublewords, and quadwords are given with the low-order byte first.

• +rb, +rw, +rd, +ro — Indicated the lower 3 bits of the opcode byte is used to encode the register operand
without a modR/M byte. The instruction lists the corresponding hexadecimal value of the opcode byte with low
3 bits as 000b. In non-64-bit mode, a register code, from 0 through 7, is added to the hexadecimal value of the
opcode byte. In 64-bit mode, indicates the four bit field of REX.b and opcode[2:0] field encodes the register
operand of the instruction. “+ro” is applicable only in 64-bit mode. See the following table for the codes.

• +i — A number used in floating-point instructions when one of the operands is ST(i) from the FPU register stack.
The number i (which can range from 0 to 7) is added to the hexadecimal byte given at the left of the plus sign
to form a single opcode byte.




byte register                       word register                       dword register                      quadword register (64-Bit Mode only)
Register    REX.B   Reg Field       Register    REX.B   Reg Field       Register    REX.B   Reg Field       Register    REX.B   Reg Field
AL          None    0               AX          None    0               EAX         None    0               RAX         None    0
CL          None    1               CX          None    1               ECX         None    1               RCX         None    1
DL          None    2               DX          None    2               EDX         None    2               RDX         None    2
BL          None    3               BX          None    3               EBX         None    3               RBX         None    3
AH          N.E.    4               SP          None    4               ESP         None    4               N/A         N/A     N/A
CH          N.E.    5               BP          None    5               EBP         None    5               N/A         N/A     N/A
DH          N.E.    6               SI          None    6               ESI         None    6               N/A         N/A     N/A
BH          N.E.    7               DI          None    7               EDI         None    7               N/A         N/A     N/A
SPL         Yes     4               SP          None    4               ESP         None    4               RSP         None    4
BPL         Yes     5               BP          None    5               EBP         None    5               RBP         None    5
SIL         Yes     6               SI          None    6               ESI         None    6               RSI         None    6
DIL         Yes     7               DI          None    7               EDI         None    7               RDI         None    7
R8B         Yes     0               R8W         Yes     0               R8D         Yes     0               R8          Yes     0
R9B         Yes     1               R9W         Yes     1               R9D         Yes     1               R9          Yes     1
Registers R8 - R15 (see below): Available in 64-Bit Mode Only
R10B        Yes     2               R10W        Yes     2               R10D        Yes     2               R10         Yes     2
R11B        Yes     3               R11W        Yes     3               R11D        Yes     3               R11         Yes     3
R12B        Yes     4               R12W        Yes     4               R12D        Yes     4               R12         Yes     4
R13B        Yes     5               R13W        Yes     5               R13D        Yes     5               R13         Yes     5
R14B        Yes     6               R14W        Yes     6               R14D        Yes     6               R14         Yes     6
R15B        Yes     7               R15W        Yes     7               R15D        Yes     7               R15         Yes     7




Example instructions:

----- EXAMPLE 1 -----

MOV %ecx, $0xfffffff5

This is in the form:
    MOV r32, imm32

Which is encoded as:
    B8+ rd id

This means adding the lower 3 bits of the register value to 0xB8, then following by the 4-byte immediate value.
This results in the following instruction:
    b9 f5 ff ff ff
    

----- EXAMPLE 2 -----

MOV %rcx, %rax

This is in the form:
    MOV r/m64, r64

Which is encoded as
    REX.W + 89 /r

This means the instruction begins with REX.W (encoded as 0x48) and 0x89, followed by a ModR/M byte

The destination is a register, so the mod value is 0b11
Reg is used for the source register, which in this case is RAX, so will be set to 0b000
Rm is used for the destination register, which in this case is RCX, so will be set to 0b001

This means the ModR/M byte is 0b11000001, or 0xc1

This results in the following instruction:
    48 89 c1

*/

void print_encoded(uint8_t *encoded, std::size_t size)
{
    std::cout << size << ": ";

    for (std::size_t i = 0; i < size; i++)
    {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)(encoded[i]) << " ";
    }

    std::cout << std::endl;
}

void encode(uint8_t *&encoded, std::size_t &size)
{
    encoded = new uint8_t[16]{};

    print_encoded(encoded, size);
}