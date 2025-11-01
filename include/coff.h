// Based on https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
// All my own code - may not contain everything necessary for the COFF format

#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdint>

#define IMAGE_FILE_MACHINE_UNKNOWN 0x0        // The content of this field is assumed to be applicable to any machine type
#define IMAGE_FILE_MACHINE_ALPHA 0x184        // Alpha AXP, 32-bit address space
#define IMAGE_FILE_MACHINE_ALPHA64 0x284      // Alpha 64, 64-bit address space
#define IMAGE_FILE_MACHINE_AM33 0x1d3         // Matsushita AM33
#define IMAGE_FILE_MACHINE_AMD64 0x8664       // x64
#define IMAGE_FILE_MACHINE_ARM 0x1c0          // ARM little endian
#define IMAGE_FILE_MACHINE_ARM64 0xaa64       // ARM64 little endian
#define IMAGE_FILE_MACHINE_ARM64EC 0xA641     // ABI that enables interoperability between native ARM64 and emulated x64 code.
#define IMAGE_FILE_MACHINE_ARM64X 0xA64E      // Binary format that allows both native ARM64 and ARM64EC code to coexist in the same file.
#define IMAGE_FILE_MACHINE_ARMNT 0x1c4        // ARM Thumb-2 little endian
#define IMAGE_FILE_MACHINE_AXP64 0x284        // AXP 64 (Same as Alpha 64)
#define IMAGE_FILE_MACHINE_EBC 0xebc          // EFI byte code
#define IMAGE_FILE_MACHINE_I386 0x14c         // Intel 386 or later processors and compatible processors
#define IMAGE_FILE_MACHINE_IA64 0x200         // Intel Itanium processor family
#define IMAGE_FILE_MACHINE_LOONGARCH32 0x6232 // LoongArch 32-bit processor family
#define IMAGE_FILE_MACHINE_LOONGARCH64 0x6264 // LoongArch 64-bit processor family
#define IMAGE_FILE_MACHINE_M32R 0x9041        // Mitsubishi M32R little endian
#define IMAGE_FILE_MACHINE_MIPS16 0x266       // MIPS16
#define IMAGE_FILE_MACHINE_MIPSFPU 0x366      // MIPS with FPU
#define IMAGE_FILE_MACHINE_MIPSFPU16 0x466    // MIPS16 with FPU
#define IMAGE_FILE_MACHINE_POWERPC 0x1f0      // Power PC little endian
#define IMAGE_FILE_MACHINE_POWERPCFP 0x1f1    // Power PC with floating point support
#define IMAGE_FILE_MACHINE_R3000BE 0x160      // MIPS I compatible 32-bit big endian
#define IMAGE_FILE_MACHINE_R3000 0x162        // MIPS I compatible 32-bit little endian
#define IMAGE_FILE_MACHINE_R4000 0x166        // MIPS III compatible 64-bit little endian
#define IMAGE_FILE_MACHINE_R10000 0x168       // MIPS IV compatible 64-bit little endian
#define IMAGE_FILE_MACHINE_RISCV32 0x5032     // RISC-V 32-bit address space
#define IMAGE_FILE_MACHINE_RISCV64 0x5064     // RISC-V 64-bit address space
#define IMAGE_FILE_MACHINE_RISCV128 0x5128    // RISC-V 128-bit address space
#define IMAGE_FILE_MACHINE_SH3 0x1a2          // Hitachi SH3
#define IMAGE_FILE_MACHINE_SH3DSP 0x1a3       // Hitachi SH3 DSP
#define IMAGE_FILE_MACHINE_SH4 0x1a6          // Hitachi SH4
#define IMAGE_FILE_MACHINE_SH5 0x1a8          // Hitachi SH5
#define IMAGE_FILE_MACHINE_THUMB 0x1c2        // Thumb
#define IMAGE_FILE_MACHINE_WCEMIPSV2 0x169    // MIPS little-endian WCE v2

#define IMAGE_FILE_RELOCS_STRIPPED 0x0001         // Image only, Windows CE, and Microsoft Windows NT and later. This indicates that the file does not contain base relocations and must therefore be loaded at its preferred base address. If the base address is not available, the loader reports an error. The default behavior of the linker is to strip base relocations from executable (EXE) files.
#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002        // Image only. This indicates that the image file is valid and can be run. If this flag is not set, it indicates a linker error.
#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004      // COFF line numbers have been removed. This flag is deprecated and should be zero.
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED 0x0008     // COFF symbol table entries for local symbols have been removed. This flag is deprecated and should be zero.
#define IMAGE_FILE_AGGRESSIVE_WS_TRIM 0x0010      // Obsolete. Aggressively trim working set. This flag is deprecated for Windows 2000 and later and must be zero.
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020     // Application can handle > 2-GB addresses.
#define IMAGE_FILE_BYTES_REVERSED_LO 0x0080       // Little endian: the least significant bit (LSB) precedes the most significant bit (MSB) in memory. This flag is deprecated and should be zero.
#define IMAGE_FILE_32BIT_MACHINE 0x0100           // Machine is based on a 32-bit-word architecture.
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200          // Debugging information is removed from the image file.
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP 0x0400 // If the image is on removable media, fully load it and copy it to the swap file.
#define IMAGE_FILE_NET_RUN_FROM_SWAP 0x0800       // If the image is on network media, fully load it and copy it to the swap file.
#define IMAGE_FILE_SYSTEM 0x1000                  // The image file is a system file, not a user program.
#define IMAGE_FILE_DLL 0x2000                     // The image file is a dynamic-link library (DLL). Such files are considered executable files for almost all purposes, although they cannot be directly run.
#define IMAGE_FILE_UP_SYSTEM_ONLY 0x4000          // The file should be run only on a uniprocessor machine.
#define IMAGE_FILE_BYTES_REVERSED_HI 0x8000       // Big endian: the MSB precedes the LSB in memory. This flag is deprecated and should be zero.

#define IMAGE_SUBSYSTEM_UNKNOWN 0                   // An unknown subsystem
#define IMAGE_SUBSYSTEM_NATIVE 1                    // Device drivers and native Windows processes
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2               // The Windows graphical user interface (GUI) subsystem
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3               // The Windows character subsystem
#define IMAGE_SUBSYSTEM_OS2_CUI 5                   // The OS/2 character subsystem
#define IMAGE_SUBSYSTEM_POSIX_CUI 7                 // The Posix character subsystem
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS 8            // Native Win9x driver
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI 9            // Windows CE
#define IMAGE_SUBSYSTEM_EFI_APPLICATION 10          // An Extensible Firmware Interface (EFI) application
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER 11  // An EFI driver with boot services
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER 12       // An EFI driver with run-time services
#define IMAGE_SUBSYSTEM_EFI_ROM 13                  // An EFI ROM image
#define IMAGE_SUBSYSTEM_XBOX 14                     // XBOX
#define IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16 // Windows boot application.

#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA 0x0020       // Image can handle a high entropy 64-bit virtual address space.
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040          // DLL can be relocated at load time.
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY 0x0080       // Code Integrity checks are enforced.
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT 0x0100             // Image is NX compatible.
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200          // Isolation aware, but do not isolate the image.
#define IMAGE_DLLCHARACTERISTICS_NO_SEH 0x0400                // Does not use structured exception (SE) handling. No SE handler may be called in this image.
#define IMAGE_DLLCHARACTERISTICS_NO_BIND 0x0800               // Do not bind the image.
#define IMAGE_DLLCHARACTERISTICS_APPCONTAINER 0x1000          // Image must execute in an AppContainer.
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER 0x2000            // A WDM driver.
#define IMAGE_DLLCHARACTERISTICS_GUARD_CF 0x4000              // Image supports Control Flow Guard.
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000 // Terminal Server aware.

#define IMAGE_SCN_TYPE_NO_PAD 0x00000008            // The section should not be padded to the next boundary. This flag is obsolete and is replaced by IMAGE_SCN_ALIGN_1BYTES. This is valid only for object files.
#define IMAGE_SCN_CNT_CODE 0x00000020               // The section contains executable code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040   // The section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080 // The section contains uninitialized data.
#define IMAGE_SCN_LNK_OTHER 0x00000100              // Reserved for future use.
#define IMAGE_SCN_LNK_INFO 0x00000200               // The section contains comments or other information. The .drectve section has this type. This is valid for object files only.
#define IMAGE_SCN_LNK_REMOVE 0x00000800             // The section will not become part of the image. This is valid only for object files.
#define IMAGE_SCN_LNK_COMDAT 0x00001000             // The section contains COMDAT data. For more information, see COMDAT Sections (Object Only). This is valid only for object files.
#define IMAGE_SCN_GPREL 0x00008000                  // The section contains data referenced through the global pointer (GP).
#define IMAGE_SCN_MEM_PURGEABLE 0x00020000          // Reserved for future use.
#define IMAGE_SCN_MEM_16BIT 0x00020000              // Reserved for future use.
#define IMAGE_SCN_MEM_LOCKED 0x00040000             // Reserved for future use.
#define IMAGE_SCN_MEM_PRELOAD 0x00080000            // Reserved for future use.
#define IMAGE_SCN_ALIGN_1BYTES 0x00100000           // Align data on a 1-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_2BYTES 0x00200000           // Align data on a 2-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_4BYTES 0x00300000           // Align data on a 4-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_8BYTES 0x00400000           // Align data on an 8-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_16BYTES 0x00500000          // Align data on a 16-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_32BYTES 0x00600000          // Align data on a 32-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_64BYTES 0x00700000          // Align data on a 64-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_128BYTES 0x00800000         // Align data on a 128-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_256BYTES 0x00900000         // Align data on a 256-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_512BYTES 0x00A00000         // Align data on a 512-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_1024BYTES 0x00B00000        // Align data on a 1024-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_2048BYTES 0x00C00000        // Align data on a 2048-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_4096BYTES 0x00D00000        // Align data on a 4096-byte boundary. Valid only for object files.
#define IMAGE_SCN_ALIGN_8192BYTES 0x00E00000        // Align data on an 8192-byte boundary. Valid only for object files.
#define IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000        // The section contains extended relocations.
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000        // The section can be discarded as needed.
#define IMAGE_SCN_MEM_NOT_CACHED 0x04000000         // The section cannot be cached.
#define IMAGE_SCN_MEM_NOT_PAGED 0x08000000          // The section is not pageable.
#define IMAGE_SCN_MEM_SHARED 0x10000000             // The section can be shared in memory.
#define IMAGE_SCN_MEM_EXECUTE 0x20000000            // The section can be executed as code.
#define IMAGE_SCN_MEM_READ 0x40000000               // The section can be read.
#define IMAGE_SCN_MEM_WRITE 0x80000000              // The section can be written to.

#define IMAGE_REL_AMD64_ABSOLUTE 0x0000 // The relocation is ignored.
#define IMAGE_REL_AMD64_ADDR64 0x0001   // The 64-bit VA of the relocation target.
#define IMAGE_REL_AMD64_ADDR32 0x0002   // The 32-bit VA of the relocation target.
#define IMAGE_REL_AMD64_ADDR32NB 0x0003 // The 32-bit address without an image base (RVA).
#define IMAGE_REL_AMD64_REL32 0x0004    // The 32-bit relative address from the byte following the relocation.
#define IMAGE_REL_AMD64_REL32_1 0x0005  // The 32-bit address relative to byte distance 1 from the relocation.
#define IMAGE_REL_AMD64_REL32_2 0x0006  // The 32-bit address relative to byte distance 2 from the relocation.
#define IMAGE_REL_AMD64_REL32_3 0x0007  // The 32-bit address relative to byte distance 3 from the relocation.
#define IMAGE_REL_AMD64_REL32_4 0x0008  // The 32-bit address relative to byte distance 4 from the relocation.
#define IMAGE_REL_AMD64_REL32_5 0x0009  // The 32-bit address relative to byte distance 5 from the relocation.
#define IMAGE_REL_AMD64_SECTION 0x000A  // The 16-bit section index of the section that contains the target. This is used to support debugging information.
#define IMAGE_REL_AMD64_SECREL 0x000B   // The 32-bit offset of the target from the beginning of its section. This is used to support debugging information and static thread local storage.
#define IMAGE_REL_AMD64_SECREL7 0x000C  // A 7-bit unsigned offset from the base of the section that contains the target.
#define IMAGE_REL_AMD64_TOKEN 0x000D    // CLR tokens.
#define IMAGE_REL_AMD64_SREL32 0x000E   // A 32-bit signed span-dependent value emitted into the object.
#define IMAGE_REL_AMD64_PAIR 0x000F     // A pair that must immediately follow every span-dependent value.
#define IMAGE_REL_AMD64_SSPAN32 0x0010  // A 32-bit signed span-dependent value that is applied at link time.

#define IMAGE_SYM_UNDEFINED 0 // The symbol record is not yet assigned a section. A value of zero indicates that a reference to an external symbol is defined elsewhere. A value of non-zero is a common symbol with a size that is specified by the value.
#define IMAGE_SYM_ABSOLUTE -1 // The symbol has an absolute (non-relocatable) value and is not an address.
#define IMAGE_SYM_DEBUG -2    // The symbol provides general type or debugging information but does not correspond to a section. Microsoft tools use this setting along with .file records (storage class FILE).

#define IMAGE_SYM_TYPE_NULL 0   // No type information or unknown base type. Microsoft tools use this setting
#define IMAGE_SYM_TYPE_VOID 1   // No valid type; used with void pointers and functions
#define IMAGE_SYM_TYPE_CHAR 2   // A character (signed byte)
#define IMAGE_SYM_TYPE_SHORT 3  // A 2-byte signed integer
#define IMAGE_SYM_TYPE_INT 4    // A natural integer type (normally 4 bytes in Windows)
#define IMAGE_SYM_TYPE_LONG 5   // A 4-byte signed integer
#define IMAGE_SYM_TYPE_FLOAT 6  // A 4-byte floating-point number
#define IMAGE_SYM_TYPE_DOUBLE 7 // An 8-byte floating-point number
#define IMAGE_SYM_TYPE_STRUCT 8 // A structure
#define IMAGE_SYM_TYPE_UNION 9  // A union
#define IMAGE_SYM_TYPE_ENUM 10  // An enumerated type
#define IMAGE_SYM_TYPE_MOE 11   // A member of enumeration (a specific value)
#define IMAGE_SYM_TYPE_BYTE 12  // A byte; unsigned 1-byte integer
#define IMAGE_SYM_TYPE_WORD 13  // A word; unsigned 2-byte integer
#define IMAGE_SYM_TYPE_UINT 14  // An unsigned integer of natural size (normally, 4 bytes)
#define IMAGE_SYM_TYPE_DWORD 15 // An unsigned 4-byte integer

#define IMAGE_SYM_DTYPE_NULL 0     // No derived type; the symbol is a simple scalar variable.
#define IMAGE_SYM_DTYPE_POINTER 1  // The symbol is a pointer to base type.
#define IMAGE_SYM_DTYPE_FUNCTION 2 // The symbol is a function that returns a base type.
#define IMAGE_SYM_DTYPE_ARRAY 3    // The symbol is an array of base type.

#define IMAGE_SYM_CLASS_END_OF_FUNCTION -1  // A special symbol that represents the end of function, for debugging purposes.
#define IMAGE_SYM_CLASS_NULL 0              // No assigned storage class.
#define IMAGE_SYM_CLASS_AUTOMATIC 1         // The automatic (stack) variable. The Value field specifies the stack frame offset.
#define IMAGE_SYM_CLASS_EXTERNAL 2          // A value that Microsoft tools use for external symbols. The Value field indicates the size if the section number is IMAGE_SYM_UNDEFINED (0). If the section number is not zero, then the Value field specifies the offset within the section.
#define IMAGE_SYM_CLASS_STATIC 3            // The offset of the symbol within the section. If the Value field is zero, then the symbol represents a section name.
#define IMAGE_SYM_CLASS_REGISTER 4          // A register variable. The Value field specifies the register number.
#define IMAGE_SYM_CLASS_EXTERNAL_DEF 5      // A symbol that is defined externally.
#define IMAGE_SYM_CLASS_LABEL 6             // A code label that is defined within the module. The Value field specifies the offset of the symbol within the section.
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL 7   // A reference to a code label that is not defined.
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT 8  // The structure member. The Value field specifies the n th member.
#define IMAGE_SYM_CLASS_ARGUMENT 9          // A formal argument (parameter) of a function. The Value field specifies the n th argument.
#define IMAGE_SYM_CLASS_STRUCT_TAG 10       // The structure tag-name entry.
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION 11  // A union member. The Value field specifies the n th member.
#define IMAGE_SYM_CLASS_UNION_TAG 12        // The Union tag-name entry.
#define IMAGE_SYM_CLASS_TYPE_DEFINITION 13  // A Typedef entry.
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC 14 // A static data declaration.
#define IMAGE_SYM_CLASS_ENUM_TAG 15         // An enumerated type tagname entry.
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM 16   // A member of an enumeration. The Value field specifies the n th member.
#define IMAGE_SYM_CLASS_REGISTER_PARAM 17   // A register parameter.
#define IMAGE_SYM_CLASS_BIT_FIELD 18        // A bit-field reference. The Value field specifies the n th bit in the bit field.
#define IMAGE_SYM_CLASS_BLOCK 100           // A .bb (beginning of block) or .eb (end of block) record. The Value field is the relocatable address of the code location.
#define IMAGE_SYM_CLASS_FUNCTION 101        // A value that Microsoft tools use for symbol records that define the extent of a function: begin function (.bf ), end function ( .ef ), and lines in function ( .lf ). For .lf records, the Value field gives the number of source lines in the function. For .ef records, the Value field gives the size of the function code.
#define IMAGE_SYM_CLASS_END_OF_STRUCT 102   // An end-of-structure entry.
#define IMAGE_SYM_CLASS_FILE 103            // A value that Microsoft tools, as well as traditional COFF format, use for the source-file symbol record. The symbol is followed by auxiliary records that name the file.
#define IMAGE_SYM_CLASS_SECTION 104         // A definition of a section (Microsoft tools use STATIC storage class instead).
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL 105   // A weak external. For more information, see Auxiliary Format 3: Weak Externals.
#define IMAGE_SYM_CLASS_CLR_TOKEN 107       // A CLR token symbol. The name is an ASCII string that consists of the hexadecimal value of the token. For more information, see CLR Token Definition (Object Only).

#define IMAGE_COMDAT_SELECT_NODUPLICATES 1 // If this symbol is already defined, the linker issues a "multiply defined symbol" error.
#define IMAGE_COMDAT_SELECT_ANY 2          // Any section that defines the same COMDAT symbol can be linked; the rest are removed.
#define IMAGE_COMDAT_SELECT_SAME_SIZE 3    // The linker chooses an arbitrary section among the definitions for this symbol. If all definitions are not the same size, a "multiply defined symbol" error is issued.
#define IMAGE_COMDAT_SELECT_EXACT_MATCH 4  // The linker chooses an arbitrary section among the definitions for this symbol. If all definitions do not match exactly, a "multiply defined symbol" error is issued.
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE 5  // The section is linked if a certain other COMDAT section is linked. This other section is indicated by the Number field of the auxiliary symbol record for the section definition. This setting is useful for definitions that have components in multiple sections (for example, code in one and data in another), but where all must be linked or discarded as a set. The other section this section is associated with must be a COMDAT section, which can be another associative COMDAT section. An associative COMDAT section's section association chain can't form a loop. The section association chain must eventually come to a COMDAT section that doesn't have #define IMAGE_COMDAT_SELECT_ASSOCIATIVE set.
#define IMAGE_COMDAT_SELECT_LARGEST 6      // The linker chooses the largest definition from among all of the definitions for this symbol. If multiple definitions have this size, the choice between them is arbitrary.

struct Name
{
    char name[8];

    Name &operator=(std::string str)
    {
        std::size_t len = str.length();
        if (len <= 8)
        {
            strncpy(name, str.c_str(), len);

            if (len < 8)
            {
                std::fill(name + len, name + 7, 0);
            }
        }

        return *this;
    }

    bool operator==(std::string str)
    {
        if (std::string(name) == str)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

struct COFF_Hdr
{
    uint16_t machine;
    uint16_t num_sections;
    uint32_t time_date;
    uint32_t sym_tab;
    uint32_t num_sym;
    uint16_t opt_size;
    uint16_t flags;
};

#pragma pack(push, 1)
struct Reloc
{
    uint32_t virt_addr;
    uint32_t sym_tab_idx;
    uint16_t type;
};
#pragma pack(pop)

struct Rel_Tab
{
    std::vector<Reloc> relocations;

    Reloc &operator[](std::size_t idx)
    {
        return relocations[idx];
    }

    std::size_t size()
    {
        return relocations.size();
    }

    void emplace_back(Reloc rel)
    {
        relocations.emplace_back(rel);
    }
};

struct Sect_Hdr
{
    Name name;
    uint32_t virt_size;
    uint32_t virt_addr;
    uint32_t raw_size;
    uint32_t data;
    uint32_t reloc;
    uint32_t line_num;
    uint16_t num_reloc;
    uint16_t num_line_num;
    uint32_t flags;
};

#pragma pack(push, 1)
struct Sym_Hdr
{
    Name name;
    uint32_t value;
    uint16_t sect_num;
    uint16_t type;
    uint8_t storage_class;
    uint8_t num_aux_sym;
};
#pragma pack(pop)

struct Section
{
    Sect_Hdr header;
    std::vector<uint8_t> data;
    Rel_Tab relocations = {};

    void append(uint8_t *to_add, std::size_t size);

    template <typename T>
    void append_literal(T to_add);

    void append(std::size_t size);

    void append(std::size_t size, uint8_t val);

    void reserve(std::size_t size);

    void align();
};

struct Sect_Tab
{
    std::vector<Section> sections;

    Section &operator[](std::size_t idx)
    {
        return sections[idx];
    }

    Section &operator[](std::string key)
    {
        std::size_t idx = 0;

        for (Section section : sections)
        {
            if (section.header.name == key)
            {
                break;
            }

            idx++;
        }

        return sections[idx];
    }

    std::size_t size()
    {
        return sections.size();
    }

    std::size_t find(std::string key)
    {
        std::size_t idx = 0;

        for (Section section : sections)
        {
            if (section.header.name == key)
            {
                return idx;
            }

            idx++;
        }

        return (std::size_t)(-1);
    }
};

struct Sym_Tab
{
    std::vector<Sym_Hdr> symbols;
    std::vector<uint8_t> *str_tab;

    Sym_Hdr &operator[](std::size_t idx)
    {
        return symbols[idx];
    }

    Sym_Hdr &operator[](std::string key)
    {
        std::size_t idx = 0;

        for (Sym_Hdr sym : symbols)
        {
            if (sym.name == key)
            {
                break;
            }

            idx++;
        }

        return symbols[idx];
    }

    std::size_t size()
    {
        return symbols.size();
    }
    
    std::size_t find(std::string key)
    {
        std::size_t idx = 0;

        for (Sym_Hdr sym : symbols)
        {
            if (sym.name.name[0] == 0)
            {
                if (std::string((char *)(&(*str_tab)[*(uint32_t *)(sym.name.name + 4)])) == key)
                {
                    return idx;
                }
            }
            else if (sym.name == key)
            {
                return idx;
            }

            idx++;
        }

        return (std::size_t)(-1);
    }

    void emplace_back(Sym_Hdr sym)
    {
        symbols.emplace_back(sym);
    }
};

#pragma pack(push, 1)
struct Aux_Form_5
{
    uint32_t length;
    uint16_t num_rel;
    uint16_t num_line_num;
    uint32_t checksum;
    uint16_t number;
    uint8_t selection;
    uint8_t reserve[3] = {};
};
#pragma pack(pop)