#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>

#include <coff.h>
#include <encoder.h>

struct Label
{
    std::string name;
    std::size_t loc;
    std::string section;
};

void add_label(std::string name, std::size_t loc, std::string section, std::vector<Label> &labels)
{
    Label label = {};

    label.name = name;
    label.loc = loc;
    label.section = section;

    labels.emplace_back(label);
}

void add_symbol(std::string symbol, Sym_Tab &sym_tab, Sect_Tab &sections, std::vector<uint8_t> &str_tab, uint8_t storage_class, std::string str = "", uint32_t value = 0, uint8_t type = IMAGE_SYM_TYPE_NULL, uint8_t dtype = IMAGE_SYM_DTYPE_NULL)
{
    Sym_Hdr sym_hdr = {};

    sym_hdr.name = symbol;

    if (sym_hdr.name.name[0] == 0)
    {
        uint32_t loc = str_tab.size();
        str_tab.insert(str_tab.end(), symbol.c_str(), symbol.c_str() + symbol.length() + 1);

        *(uint32_t *)(sym_hdr.name.name + 4) = loc;
        *(uint32_t *)(&(str_tab[0])) += symbol.length() + 1;
    }

    if (storage_class == IMAGE_SYM_CLASS_FILE)
    {
        sym_hdr.value = 0;
        sym_hdr.sect_num = -2;
        sym_hdr.type = 0;
        sym_hdr.storage_class = storage_class;
        sym_hdr.num_aux_sym = 1;

        sym_tab.emplace_back(sym_hdr);

        memset(&sym_hdr, 0, sizeof(Sym_Hdr));
        strcpy((char *)(&sym_hdr), str.c_str());
        sym_tab.emplace_back(sym_hdr);
    }
    else if (storage_class == IMAGE_SYM_CLASS_STATIC)
    {
        sym_hdr.value = 0;
        sym_hdr.sect_num = sections.find(symbol) + 1;
        sym_hdr.type = 0;
        sym_hdr.storage_class = storage_class;
        sym_hdr.num_aux_sym = 1;

        sym_tab.emplace_back(sym_hdr);

        sym_hdr = {};
        sym_tab.emplace_back(sym_hdr);
    }
    else
    {
        uint16_t sect_num = 0;

        if (str != ".extern")
        {
            sect_num = sections.find(str) + 1;
        }

        sym_hdr.value = value;
        sym_hdr.sect_num = sect_num;
        sym_hdr.type = (dtype << 8) | type;
        sym_hdr.storage_class = storage_class;
        sym_hdr.num_aux_sym = 0;

        sym_tab.emplace_back(sym_hdr);
    }
}

void add_section(Sect_Tab &sections, Sym_Tab &sym_tab, std::vector<uint8_t> &str_tab, Sect_Hdr header)
{
    Section section = {};

    header.raw_size = 0;

    section.header = header;
    section.data = {};

    sections.sections.emplace_back(section);

    add_symbol(header.name.name, sym_tab, sections, str_tab, IMAGE_SYM_CLASS_STATIC);
}

void relocate_symbol(std::string symbol, std::string section, Sect_Tab &sections, Sym_Tab &sym_tab, uint32_t virt_addr, uint16_t type)
{
    Reloc reloc = {};

    reloc.virt_addr = virt_addr;
    reloc.sym_tab_idx = sym_tab.find(symbol);
    reloc.type = type;

    sections[section].relocations.emplace_back(reloc);
}

int main()
{
    // Initialise data

    COFF_Hdr header = {};
    Sect_Tab sections = {};
    Sect_Hdr section_header = {};
    std::vector<Label> labels = {};
    Sym_Tab sym_tab = {};
    Sym_Hdr symbol = {};
    std::vector<uint8_t> str_tab = {0x4, 0x0, 0x0, 0x0};
    uint8_t *data;
    std::vector<uint8_t> complete_data = {};

    std::size_t coff_header_size = 0;
    std::size_t section_data = 0;
    std::size_t rel_tab_loc = 0;
    std::size_t sym_tab_loc = 0;

    sym_tab.str_tab = &str_tab;

    // COFF Header

    header.machine = IMAGE_FILE_MACHINE_AMD64;
    header.num_sections = 0x04;
    header.time_date = 0x00;
    header.num_sym = 0x14;
    header.opt_size = 0x00;
    header.flags = IMAGE_FILE_LINE_NUMS_STRIPPED;

    coff_header_size += sizeof(header);

    // Default sections

    section_header.name = ".text";
    section_header.flags = IMAGE_SCN_CNT_CODE | IMAGE_SCN_ALIGN_16BYTES | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;
    add_section(sections, sym_tab, str_tab, section_header);

    section_header.name = ".data";
    section_header.flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_ALIGN_16BYTES | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
    add_section(sections, sym_tab, str_tab, section_header);

    section_header.name = ".bss";
    section_header.flags = IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_ALIGN_16BYTES | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
    add_section(sections, sym_tab, str_tab, section_header);

    // Rest of code

    add_symbol(".file", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_FILE, "main.c");

    add_symbol("__imp_GetStdHandle", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".extern");
    add_symbol("__imp_WriteConsoleA", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".extern");
    add_symbol("__imp_ExitProcess", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".extern");

    add_symbol("std_out", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".bss", 0);
    add_label("std_out", 0, ".bss", labels);
    sections[".bss"].reserve(0x8);

    section_header.name = ".rdata";
    section_header.flags = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_ALIGN_16BYTES | IMAGE_SCN_MEM_READ;
    add_section(sections, sym_tab, str_tab, section_header);

    add_label(".LC0", 0, ".rdata", labels);
    sections[".rdata"].append((uint8_t *)"Hello world\n", 13);

    add_symbol("str", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".data", 0);
    add_label("str", 0, ".data", labels);
    sections[".data"].append_literal(0x0llu);
    relocate_symbol(".rdata", ".data", sections, sym_tab, 0x0, IMAGE_REL_AMD64_ADDR64);

    add_symbol("len", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".data", 8);
    add_label("len", 8, ".data", labels);
    sections[".data"].append_literal(12l);

    add_symbol("main", sym_tab, sections, str_tab, IMAGE_SYM_CLASS_EXTERNAL, ".text", 0);
    add_label("main", 0, ".text", labels);

    // Line 8: int main()
    sections[".text"].append((uint8_t *)"\x55", 1);             // pushq	%rbp
    sections[".text"].append((uint8_t *)"\x48\x89\xe5", 3);     // movq	%rsp, %rbp
    sections[".text"].append((uint8_t *)"\x48\x83\xec\x40", 4); // subq	$64, %rsp

    // Line 10: std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    sections[".text"].append((uint8_t *)"\xb9\xf5\xff\xff\xff", 5);                                                                // movl	$-11, %ecx
    relocate_symbol("__imp_GetStdHandle", ".text", sections, sym_tab, 0x3 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32); //      RELOCATION: __imp_GetStdHandle
    sections[".text"].append((uint8_t *)"\x48\x8b\x05\x00\x00\x00\x00", 7);                                                        // movq	__imp_GetStdHandle(%rip), %rax
    sections[".text"].append((uint8_t *)"\xff\xd0", 2);                                                                            // call	*%rax
    relocate_symbol(".bss", ".text", sections, sym_tab, 0x3 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32);               //      RELOCATION: std_out
    sections[".text"].append((uint8_t *)"\x48\x89\x05\x00\x00\x00\x00", 7);                                                        // movq	%rax, std_out(%rip)

    // Line 12: WriteConsoleA(std_out, str, len, NULL, NULL);
    relocate_symbol(".bss", ".text", sections, sym_tab, 0x3 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32);                //      RELOCATION: std_out
    sections[".text"].append((uint8_t *)"\x48\x8b\x05\x00\x00\x00\x00", 7);                                                         // movq	std_out(%rip), %rax
    sections[".text"].append((uint8_t *)"\x48\x89\xc1", 3);                                                                         // movq	%rax, %rcx
    relocate_symbol(".data", ".text", sections, sym_tab, 0x3 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32);               //      RELOCATION: str
    sections[".text"].append((uint8_t *)"\x48\x8b\x15\x00\x00\x00\x00", 7);                                                         // movq	str(%rip), %rdx
    relocate_symbol(".data", ".text", sections, sym_tab, 0x2 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32);               //      RELOCATION: len
    sections[".text"].append((uint8_t *)"\x8b\x05\x08\x00\x00\x00", 6);                                                             // movl	len(%rip), %eax
    sections[".text"].append((uint8_t *)"\x41\x89\xc0", 3);                                                                         // movl	%eax, %r8d
    sections[".text"].append((uint8_t *)"\x41\xb9\x00\x00\x00\x00", 6);                                                             // movl	$0, %r9d
    sections[".text"].append((uint8_t *)"\x48\xc7\x44\x24\x20\x00\x00\x00\x00", 9);                                                 // movq	$0, 32(%rsp)
    relocate_symbol("__imp_WriteConsoleA", ".text", sections, sym_tab, 0x3 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32); //      RELOCATION: __imp_WriteConsoleA
    sections[".text"].append((uint8_t *)"\x48\x8b\x05\x00\x00\x00\x00", 7);                                                         // movq	__imp_WriteConsoleA(%rip), %rax
    sections[".text"].append((uint8_t *)"\xff\xd0", 2);                                                                             // call	*%rax

    // Line 14: ExitProcess(0);
    sections[".text"].append((uint8_t *)"\xb9\x00\x00\x00\x00", 5);                                                               // movl	$0, %ecx
    relocate_symbol("__imp_ExitProcess", ".text", sections, sym_tab, 0x3 + sections[".text"].data.size(), IMAGE_REL_AMD64_REL32); //      RELOCATION: __imp_ExitProcess
    sections[".text"].append((uint8_t *)"\x48\x8b\x05\x00\x00\x00\x00", 7);                                                       // movq	__imp_ExitProcess(%rip), %rax
    sections[".text"].append((uint8_t *)"\xff\xd0", 2);                                                                           // call	*%rax

    // Adjust all file offsets

    for (std::size_t i = 0; i < sections.size(); i++)
    {
        Aux_Form_5 aux = {};

        aux.length = sections[i].header.raw_size;
        aux.num_rel = sections[i].relocations.size();

        memcpy(&(sym_tab[sym_tab.find(sections[i].header.name.name) + 1]), &aux, sizeof(Sym_Hdr));
    }

    section_data = coff_header_size + sizeof(Sect_Hdr) * header.num_sections;
    for (std::size_t i = 0; i < header.num_sections; i++)
    {
        if (sections[i].data.size() > 0)
        {
            sections[i].align();
            sections[i].header.data = section_data;
            section_data += sections[i].header.raw_size;
        }
    }

    rel_tab_loc = section_data;
    for (std::size_t i = 0; i < sections.size(); i++)
    {
        if (sections[i].relocations.size() > 0)
        {
            sections[i].header.reloc = rel_tab_loc;
            sections[i].header.num_reloc = sections[i].relocations.size();
            rel_tab_loc += sections[i].relocations.size() * sizeof(Reloc);
        }
    }

    header.sym_tab = rel_tab_loc;
    header.num_sym = sym_tab.size();

    // Setup a single buffer to send data to the file

    complete_data.insert(complete_data.end(), (uint8_t *)(&header), (uint8_t *)(&header) + sizeof(header));

    for (std::size_t i = 0; i < sections.size(); i++)
    {
        uint8_t *addr = (uint8_t *)(&(sections[i].header));
        complete_data.insert(complete_data.end(), addr, addr + sizeof(Sect_Hdr));
    }

    for (std::size_t i = 0; i < sections.size(); i++)
    {
        if (sections[i].data.size() > 0)
        {
            complete_data.insert(complete_data.end(), sections[i].data.begin(), sections[i].data.end());
        }
    }

    for (std::size_t i = 0; i < sections.size(); i++)
    {
        if (sections[i].relocations.size() > 0)
        {
            for (std::size_t j = 0; j < sections[i].relocations.size(); j++)
            {
                uint8_t *addr = (uint8_t *)(&(sections[i].relocations[j]));
                complete_data.insert(complete_data.end(), addr, addr + sizeof(Reloc));
            }
        }
    }

    for (std::size_t i = 0; i < sym_tab.size(); i++)
    {
        complete_data.insert(complete_data.end(), (uint8_t *)(&(sym_tab[i])), (uint8_t *)(&(sym_tab[i])) + sizeof(Sym_Hdr));
    }

    for (std::size_t i = 0; i < str_tab.size(); i++)
    {
        complete_data.insert(complete_data.end(), str_tab[i]);
    }

    // Write to the output binary file

    std::ofstream fs("test\\main.obj", std::ios::out | std::ios::binary);
    fs.write((const char *)(complete_data.data()), complete_data.size());
    fs.close();

    uint8_t *encoded_instruction = NULL;
    std::size_t encoded_size;
    encode(encoded_instruction, encoded_size);
}