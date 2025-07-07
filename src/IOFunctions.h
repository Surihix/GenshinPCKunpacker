#pragma once

bool DeleteDir(const wchar_t* directory);

std::wstring CharToWString(char* chara);

void CreateDirectoryClean(const std::string& directory);

void CreateDirectoryNormal(const std::string& directory);