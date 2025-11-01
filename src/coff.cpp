#include <coff.h>

void Section::append(uint8_t *to_add, std::size_t size)
{
    if (size > 0 && to_add)
    {
        data.insert(data.end(), to_add, to_add + size);
    }

    if (data.size() >= header.raw_size)
    {
        header.raw_size += size;
    }
}

template <typename T>
void Section::append_literal(T to_add)
{
    data.insert(data.end(), (uint8_t *)(&to_add), (uint8_t *)(&to_add) + sizeof(T));

    if (data.size() >= header.raw_size)
    {
        header.raw_size += sizeof(T);
    }
}

template void Section::append_literal<uint8_t>(uint8_t);
template void Section::append_literal<uint16_t>(uint16_t);
template void Section::append_literal<uint32_t>(uint32_t);
template void Section::append_literal<uint64_t>(uint64_t);
template void Section::append_literal<unsigned long>(unsigned long);

template void Section::append_literal<int8_t>(int8_t);
template void Section::append_literal<int16_t>(int16_t);
template void Section::append_literal<int32_t>(int32_t);
template void Section::append_literal<int64_t>(int64_t);
template void Section::append_literal<long>(long);

template void Section::append_literal<uint8_t *>(uint8_t *);
template void Section::append_literal<uint16_t *>(uint16_t *);
template void Section::append_literal<uint32_t *>(uint32_t *);
template void Section::append_literal<uint64_t *>(uint64_t *);
template void Section::append_literal<unsigned long *>(unsigned long *);

template void Section::append_literal<int8_t *>(int8_t *);
template void Section::append_literal<int16_t *>(int16_t *);
template void Section::append_literal<int32_t *>(int32_t *);
template void Section::append_literal<int64_t *>(int64_t *);
template void Section::append_literal<long *>(long *);

template void Section::append_literal<void *>(void *);

void Section::append(std::size_t size)
{
    if (size > 0)
    {
        data.resize(data.size() + size, 0);
    }

    if (data.size() >= header.raw_size)
    {
        header.raw_size += size;
    }
}

void Section::append(std::size_t size, uint8_t val)
{
    if (size > 0)
    {
        data.resize(data.size() + size, val);
    }

    if (data.size() >= header.raw_size)
    {
        header.raw_size += size;
    }
}

void Section::reserve(std::size_t size)
{
    header.raw_size += size;
}

void Section::align()
{
    uint16_t alignment;

    alignment = (header.flags >> 20) & 0xf;
    alignment = 0x1 << (alignment - 1);

    uint16_t padding;

    padding = data.size() % alignment;

    if (padding > 0)
    {
        padding = alignment - padding;

        if ((header.flags >> 5) & 0x1)
        {
            append(padding, 0x90);
        }
        else
        {
            append(padding);
        }
    }
}