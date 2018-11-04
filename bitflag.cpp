#include "bitflag.h"

#include <cstring>
#include <iostream>

struct bitflag_private
{
    size_t size;
    size_t enabledFlagsCount;
    void* bits;
};

bitflag::bitflag(size_t size)
    : data(std::make_unique<bitflag_private>())
{
    data->size = size;
    data->enabledFlagsCount = 0;
    auto res = std::div(data->size, 8);
    size_t bytes = res.quot;
    bytes += res.rem > 0 ? 1 : 0;
    data->bits = std::calloc(bytes, sizeof(char));
}

bitflag::bitflag(const bitflag& other)
    : data(std::make_unique<bitflag_private>())
{
    data->size = other.data->size;
    data->enabledFlagsCount = other.data->enabledFlagsCount;
    auto res = std::div(data->size, 8);
    size_t bytes = res.quot;
    bytes += res.rem > 0 ? 1 : 0;
    data->bits = std::malloc(bytes);
    memcpy(data->bits, other.data->bits, bytes);
}

bitflag::bitflag(bitflag&& other)
    : data(std::move(other.data))
{
}

bitflag::~bitflag()
{
	if (data) {
		std::free(data->bits);
	}
}

bool bitflag::at(size_t pos) const
{
    assert(pos < data->size);
    auto res = std::div(pos, 8);
    size_t byteOffset = res.quot;
    size_t restOffset = res.rem;
    char* byte = (char*)data->bits;
    byte += byteOffset;
    return *byte & (1 << restOffset);
}

void bitflag::set(size_t pos, bool value)
{
    assert(pos < data->size);
    auto res = std::div(pos, 8);
    char* byte = (char*)data->bits;
    byte += res.quot;
    switch (value)
    {
    case true: *byte |= (1 << res.rem); ++data->enabledFlagsCount; break;
    case false: *byte &= ~(1 << res.rem); --data->enabledFlagsCount; break;
    }
}

size_t bitflag::size() const { return data->size; }

void bitflag::resize(size_t size)
{
    auto res = std::div(size, 8);
    size_t bytes = res.quot;
    bytes += res.rem > 0 ? 1 : 0;
    data->bits = realloc(data->bits, bytes);
	size_t oldSize = data->size;
	data->size = size;
	for (size_t i=oldSize; i<size; ++i)
	{
		set(i, false);
	}

}

size_t bitflag::enabled_flags_count() const { return data->enabledFlagsCount; }

bool bitflag::operator&(const bitflag& rhs) const
{
    assert(data->size >= rhs.data->size);
    size_t bitsToCheck = std::min(data->size, rhs.data->size);
    auto res = std::div(bitsToCheck, 8);
    size_t bytes = res.quot;
    bytes += res.rem > 0 ? 1 : 0;
    char* byte = (char*)data->bits;
    char* rhsByte = (char*)rhs.data->bits;
    for (int i=0; i<bytes-1; ++i)
    {
		if (bitsToCheck > 8) {
			bitsToCheck -= 8;
		}
		else {
			bitsToCheck = 0;
		}

        if (*byte == 0 && *rhsByte == 0)
        {
            ++byte;
            ++rhsByte;
            continue;
        }
        if (!(*byte & *rhsByte))
        {
            return false;
        }
        ++byte;
        ++rhsByte;
    }

	if (bitsToCheck == 0) {
		return true;
	}

    char mask = 0;
    for (size_t i=0; i<bitsToCheck; ++i)
    {
        mask |= (1 << i);
    }
	char val1 = *byte & mask;
	char val2 = *rhsByte & mask;
    return val1 == val2;
}

bitflag bitflag::operator!() const
{
	bitflag res(*this);
	for (size_t i = 0; i < res.size(); ++i) {
		res.set(i, !res.at(i));
	}
	return res;
}