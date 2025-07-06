#include <cstdint>
#include <fstream>

void ReadBytesUInt32(uint32_t& val, std::ifstream& stream)
{
	if (!stream.read(reinterpret_cast<char*>(&val), 4))
	{
		throw std::logic_error("ReadUInt32 failure!");
	}
}

void ReadBytesUInt16(uint16_t& val, std::ifstream& stream)
{
	if (!stream.read(reinterpret_cast<char*>(&val), 2))
	{
		throw std::logic_error("ReadUInt16 failure!");
	}
}