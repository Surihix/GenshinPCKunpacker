#pragma once
#include <fstream>

int InitiateUnpack(std::ifstream& stream, std::string& file);

void ParseHeader(std::ifstream& stream);

void ParsePCKCategoryChunk(std::ifstream& stream);

void UnpackFromPCK(std::ifstream& stream);