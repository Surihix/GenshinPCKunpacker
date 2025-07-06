#pragma once
#include <fstream>

int InitiateUnpack(std::ifstream& stream, std::string& file);

int ParseHeader(std::ifstream& stream);

int ParsePCKCategoryChunk(std::ifstream& stream);

void UnpackFileFromTable(std::ifstream& pckFile, std::string& outFile, uint32_t& offset, uint32_t size);

int UnpackBNK(std::ifstream& stream);

int UnpackWEM(std::ifstream& stream);

int UnpackWEMType2(std::ifstream& stream);