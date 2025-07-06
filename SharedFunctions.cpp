#include <direct.h>
#include <iostream>
#include <Windows.h>
#include <vector>

bool DeleteDir(const wchar_t* directory)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];

	//Specify a file mask. *.* = We want everything!
	wsprintf(sPath, L"%s\\*.*", directory);

	if ((hFind = FindFirstFileW(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		// Find first file will always return "."
		// and ".." as the first two directories. 
		if (wcscmp(fdFile.cFileName, L".") != 0 && wcscmp(fdFile.cFileName, L"..") != 0)
		{
			// Build up our file path using the passed in 
			// [directory] and the file/foldername we just found: 
			wsprintf(sPath, L"%s\\%s", directory, fdFile.cFileName);

			// Is the entity a File or Folder? 
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				DeleteDir(sPath); // Recursion, I love it!
			}
			else
			{
				_wremove(sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile)); // Find the next file. 

	return true;
}

const wchar_t* CharToWChar(char* chara)
{
	wchar_t* wchara = new wchar_t[MAX_PATH];

	size_t outSize;
	mbstowcs_s(&outSize, wchara, MAX_PATH, chara, (size_t)strlen(chara));

	return wchara;
}

void CreateDir(const char* directory)
{
	// Try creating a directory
	int status = _mkdir(directory);

	if (status == -1)
	{
		// Delete directory as it exists
		status = _rmdir(directory);

		// If failed, assume its not empty
		if (status == -1)
		{
			DeleteDir(CharToWChar((char*)directory));

			// Try deleting the directory again
			status = _rmdir(directory);

			if (status == -1)
			{
				throw std::logic_error("Unable to delete directory!");
			}
		}

		// Try creating the directory again
		status = _mkdir(directory);

		if (status == -1)
		{
			throw std::logic_error("Unable to create directory!");
		}
	}
}