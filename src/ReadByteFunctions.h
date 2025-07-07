#pragma once

#include <fstream>

void ReadBytesUInt32(uint32_t& val, std::ifstream& stream);

void ReadBytesUInt16(uint16_t& val, std::ifstream& stream);

void ReadBytesUInt64(uint64_t& val, std::ifstream& stream);