#include <fstream>
#include <iostream>
#include "ReadByteFunctions.h"
#include "IOFunctions.h"
#include <string>
#include <cstdio>
#include <map>

typedef struct Header
{
    char magic[4];
    uint32_t dataStartRelative;
    uint32_t unkVal;
    uint32_t pckCategoryChunkSize;
    uint32_t bnkEntryTableSize;
    uint32_t wemEntryTableSize;
    uint32_t wemType2EntryTableSize;
};

typedef struct PCKCategoryEntryTable
{
    uint32_t offset;
    uint32_t id;
};

typedef struct BNKEntryTable
{
    uint32_t id;
    uint32_t unkVal;
    uint32_t size;
    uint32_t offset;
    uint32_t categoryId;
};

typedef struct WEMEntryTable
{
    uint32_t id;
    uint32_t unkVal;
    uint32_t size;
    uint32_t offset;
    uint32_t categoryId;
};

typedef struct WEMType2EntryTable
{
    uint64_t id;
    uint32_t unkVal;
    uint32_t size;
    uint32_t offset;
    uint32_t categoryId;
};

std::string extractDir;
std::map<uint32_t, std::string> categoryDict;
std::string categoryName;
std::string outFile;
std::streampos currentPos;

Header header{};
PCKCategoryEntryTable pckCategoryEntryTable{};
BNKEntryTable bnkEntryTable{};
WEMEntryTable wemEntryTable{};
WEMType2EntryTable wemType2EntryTable{};


int ParseHeader(std::ifstream& pckFile)
{
    pckFile.read(reinterpret_cast<char*>(&header.magic), 4);
    if (strncmp(header.magic, "AKPK", 4) != 0)
    {
        std::cout << "Error: PCK file is invalid!";
        return -1;
    }

    ReadBytesUInt32(header.dataStartRelative, pckFile);
    ReadBytesUInt32(header.unkVal, pckFile);
    ReadBytesUInt32(header.pckCategoryChunkSize, pckFile);
    ReadBytesUInt32(header.bnkEntryTableSize, pckFile);
    ReadBytesUInt32(header.wemEntryTableSize, pckFile);
    ReadBytesUInt32(header.wemType2EntryTableSize, pckFile);

    std::cout << "DataStart (relative): " << header.dataStartRelative << "\n";
    std::cout << "PCKCategoryChunk size: " << header.pckCategoryChunkSize << "\n";
    std::cout << "BNK Entrytable size: " << header.bnkEntryTableSize << "\n";
    std::cout << "WEM Entrytable size: " << header.wemEntryTableSize << "\n";
    std::cout << "WEM-Type2 Entrytable size: " << header.wemType2EntryTableSize << "\n";

    return 0;
}


int ParsePCKCategoryChunk(std::ifstream& pckFile)
{
    uint32_t pckCategoryCount;
    ReadBytesUInt32(pckCategoryCount, pckFile);

    std::cout << "\n";
    std::cout << "PCKCategoryCount: " << pckCategoryCount << "\n";

    uint32_t categoryChunkAmountRead = 4;

    for (uint32_t i = 0; i < pckCategoryCount; i++)
    {
        ReadBytesUInt32(pckCategoryEntryTable.offset, pckFile);
        pckCategoryEntryTable.offset += 28;

        ReadBytesUInt32(pckCategoryEntryTable.id, pckFile);
        categoryChunkAmountRead += 8;

        currentPos = pckFile.tellg();
        pckFile.seekg(pckCategoryEntryTable.offset);

        uint8_t categorySize = 0;
        uint8_t b = 0;
        uint32_t pos = pckCategoryEntryTable.offset;
        uint16_t b2;
        std::string category;

        while (true)
        {
            pckFile.read(reinterpret_cast<char*>(&b), 1);

            if (b == 0)
            {
                pos++;
                categoryChunkAmountRead++;
                ReadBytesUInt16(b2, pckFile);

                if (b2 == 0)
                {
                    categoryChunkAmountRead += 2;
                    break;
                }
                else
                {
                    pckFile.seekg(pos);
                }
            }
            else
            {
                category += b;
                pos++;
                categoryChunkAmountRead++;
            }
        }

        std::cout << category << " " << pckCategoryEntryTable.id << "\n";
        categoryDict.insert({ pckCategoryEntryTable.id, category});

        if (i != pckCategoryCount - 1)
        {
            pckFile.seekg(currentPos);
        }
    }

    currentPos = pckFile.tellg();
    uint32_t categoryChunkRemaining = header.pckCategoryChunkSize - categoryChunkAmountRead;

    if (categoryChunkRemaining != 0)
    {
        currentPos += categoryChunkRemaining;
        pckFile.seekg(currentPos);
        currentPos = pckFile.tellg();
    }

    std::cout << "\n";

    return 0;
}


void UnpackFileFromTable(std::ifstream& pckFile, std::string& outFile, uint32_t& offset, uint32_t& size)
{
    if (remove(outFile.c_str()) == 0)
    {
        std::cout << "File exists! will be replaced!" << "\n";
    }

    std::ofstream trackFile(outFile, std::ofstream::binary);
    pckFile.seekg(offset);

    char* buffer = new char[size];
    pckFile.read(buffer, size);
    trackFile.write(buffer, size);
    delete[] buffer;
    trackFile.close();

    std::ifstream checkStream(outFile, std::ifstream::binary);
    if (checkStream.good())
    {
        checkStream.close();
        std::cout << "Unpacked file: " << outFile << "\n\n";
    }
    else
    {
        checkStream.close();
        std::cout << "Failed to unpack file: " << outFile << "\n\n";
    }
}


int UnpackBNK(std::ifstream& pckFile)
{
    uint32_t bnkCount;
    ReadBytesUInt32(bnkCount, pckFile);
    std::cout << "Bank Count: " << bnkCount << "\n\n";

    for (uint32_t i = 0; i < bnkCount; i++)
    {
        ReadBytesUInt32(bnkEntryTable.id, pckFile);
        ReadBytesUInt32(bnkEntryTable.unkVal, pckFile);
        ReadBytesUInt32(bnkEntryTable.size, pckFile);
        ReadBytesUInt32(bnkEntryTable.offset, pckFile);
        ReadBytesUInt32(bnkEntryTable.categoryId, pckFile);

        currentPos = pckFile.tellg();

        std::cout << "ID: " << bnkEntryTable.id << "\n";
        std::cout << "Size: " << bnkEntryTable.size << "\n";
        std::cout << "Offset: " << bnkEntryTable.offset << "\n";

        categoryName = categoryDict[bnkEntryTable.categoryId];
        std::cout << "Category: " << categoryName << "\n";

        outFile = extractDir + "\\" + categoryName;
        CreateDirectoryNormal(outFile);

        outFile += "\\" + std::to_string(bnkEntryTable.id) + ".bnk";

        UnpackFileFromTable(pckFile, outFile, bnkEntryTable.offset, bnkEntryTable.size);

        pckFile.seekg(currentPos);
    }

    return 0;
}


int UnpackWEM(std::ifstream& pckFile)
{
    uint32_t trackCount;
    ReadBytesUInt32(trackCount, pckFile);
    std::cout << "Track Count: " << trackCount << "\n\n";

    for (uint32_t i = 0; i < trackCount; i++)
    {
        ReadBytesUInt32(wemEntryTable.id, pckFile);
        ReadBytesUInt32(wemEntryTable.unkVal, pckFile);
        ReadBytesUInt32(wemEntryTable.size, pckFile);
        ReadBytesUInt32(wemEntryTable.offset, pckFile);
        ReadBytesUInt32(wemEntryTable.categoryId, pckFile);

        currentPos = pckFile.tellg();

        std::cout << "ID: " << wemEntryTable.id << "\n";
        std::cout << "Size: " << wemEntryTable.size << "\n";
        std::cout << "Offset: " << wemEntryTable.offset << "\n";

        categoryName = categoryDict[wemEntryTable.categoryId];
        std::cout << "Category: " << categoryName << "\n";

        outFile = extractDir + "\\" + categoryName;
        CreateDirectoryNormal(outFile);

        outFile += "\\" + std::to_string(wemEntryTable.id) + ".wem";

        UnpackFileFromTable(pckFile, outFile, wemEntryTable.offset, wemEntryTable.size);

        pckFile.seekg(currentPos);
    }

    return 0;
}


int UnpackWEMType2(std::ifstream& pckFile)
{
    uint32_t trackCount;
    ReadBytesUInt32(trackCount, pckFile);
    std::cout << "Track Count: " << trackCount << "\n\n";

    for (uint32_t i = 0; i < trackCount; i++)
    {
        ReadBytesUInt64(wemType2EntryTable.id, pckFile);
        ReadBytesUInt32(wemType2EntryTable.unkVal, pckFile);
        ReadBytesUInt32(wemType2EntryTable.size, pckFile);
        ReadBytesUInt32(wemType2EntryTable.offset, pckFile);
        ReadBytesUInt32(wemType2EntryTable.categoryId, pckFile);

        currentPos = pckFile.tellg();

        std::cout << "ID: " << wemType2EntryTable.id << "\n";
        std::cout << "Size: " << wemType2EntryTable.size << "\n";
        std::cout << "Offset: " << wemType2EntryTable.offset << "\n";

        categoryName = categoryDict[wemType2EntryTable.categoryId];
        std::cout << "Category: " << categoryName << "\n";

        outFile = extractDir + "\\" + categoryName;
        CreateDirectoryNormal(outFile);

        outFile += "\\" + std::to_string(wemType2EntryTable.id) + ".wem";

        UnpackFileFromTable(pckFile, outFile, wemType2EntryTable.offset, wemType2EntryTable.size);

        pckFile.seekg(currentPos);
    }

    return 0;
}


int InitiateUnpack(std::ifstream& pckFile, std::string& file)
{
    // Prep before extraction
    std::cout << "PCKFile: " << file <<"\n\n";
    if (file.find("\\") != std::string::npos)
    {
        std::string dir = file.substr(0, file.find_last_of("\\"));
        std::string fileName = file.substr(file.find_last_of("\\") + 1);
        extractDir = dir + "\\_" + fileName;
    }
    else
    {
        extractDir = "_";
        extractDir += file;
    }

    CreateDirectoryClean(extractDir);

    // Read Header data
    if (ParseHeader(pckFile) == -1)
    {
        return -1;
    }
    
    // Read PCKCategoryChunk data
    if (ParsePCKCategoryChunk(pckFile) == -1)
    {
        return -1;
    }

    // Read BNKEntryTable data
    if (header.bnkEntryTableSize != 4)
    {
        if (UnpackBNK(pckFile) == -1)
        {
            return -1;
        }
    }
    else
    {
        currentPos = pckFile.tellg();
        currentPos += 4;
        pckFile.seekg(currentPos);
    }

    // Read WEMEntryTable data
    if (header.wemEntryTableSize != 4)
    {
        if (UnpackWEM(pckFile) == -1)
        {
            return -1;
        }
    }
    else
    {
        currentPos = pckFile.tellg();
        currentPos += 4;
        pckFile.seekg(currentPos);
    }

    // Read WEMType2EntryTable data
    if (header.wemType2EntryTableSize != 4)
    {
        if (UnpackWEMType2(pckFile) == -1)
        {
            return -1;
        }
    }
    else
    {
        currentPos = pckFile.tellg();
        currentPos += 4;
        pckFile.seekg(currentPos);
    }

    std::cout << "\nFinished unpacking pck file!\n";

    return 0;
}