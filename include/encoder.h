#include <cstdint>

#define REX_PRESENT 0x40 // Whether REX byte is present
#define REX_W 0x8        // 1 = 64 Bit Operand Size
#define REX_R 0x4        // Extension of the ModR/M reg field
#define REX_X 0x2        // Extension of the SIB index field
#define REX_B 0x1        // Extension of the ModR/M r/m field, SIB base field, or Opcode reg field

struct Instruction
{
    std::string name;
    uint8_t prefix;
    uint8_t REX;
    uint32_t opcode;
};

void encode(uint8_t *&encoded, std::size_t &size);