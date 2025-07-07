#include <direct.h>
#include <iostream>
#include <Windows.h>

bool DeleteDir(const wchar_t* directory)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	wchar_t sPath[2048];

	// Specify a file mask. *.* = We want everything!
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
				int delStatus = _wrmdir(sPath);

				if (delStatus != 0)
				{
					bool isEmptied = DeleteDir(sPath); // Recursion. run this function inside this directory.
					
					if (isEmptied)
					{
						delStatus = _wrmdir(sPath);

						if (delStatus != 0)
						{
							throw std::logic_error("Unable to delete directory. exception in loop!");
						}
					}
				}
			}
			else
			{
				_wremove(sPath);
			}
		}
	} while (FindNextFile(hFind, &fdFile)); // Find the next file. 

	return true;
}


std::wstring CharToWString(const char* chara)
{
	wchar_t wchara[MAX_PATH];

	size_t outSize;
	mbstowcs_s(&outSize, wchara, MAX_PATH, chara, (size_t)strlen(chara));

	return std::wstring(wchara);
}


void CreateDirectoryClean(const std::string& directory)
{
	int status;
	const char* dir = directory.c_str();

	// Try creating the directory
	status = _mkdir(dir);

	if (status != 0)
	{
		// Assume directory exists, try deleting
		status = _rmdir(dir);

		if (status != 0)
		{
			// If failed, assume its not empty
			DeleteDir(CharToWString(dir).c_str());

			// Try deleting the directory again
			status = _rmdir(dir);

			if (status != 0)
			{
				throw std::logic_error("Unable to delete directory!");
			}
		}
	}
}


void CreateDirectoryNormal(const std::string& directory)
{
	int status = _mkdir(directory.c_str());
}