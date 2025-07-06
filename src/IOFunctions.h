#pragma once

bool DeleteDir(const wchar_t* sDir);

const wchar_t* CharToWChar(char* chara);

void CreateDirCustom(const char* directory, bool &shouldDeleteOld);