#include <fstream>
#include <iostream>
#include "ReadByteFunctions.h"
#include "IOFunctions.h"
#include <string>
#include <cstdio>

std::string extractDir;
bool dirDel;
uint32_t pckCategoryChunkSize = 0;
std::string* categoryBuffer;
bool isBankPCK;
uint32_t pckCategoryCount;

typedef struct PCKCategoryEntry
{
    uint32_t offset;
    uint32_t id;
};

typedef struct TrackOrBankEntry
{
    uint32_t id;
    uint32_t unkVal;
    uint32_t size;
    uint32_t offset;
    uint32_t categoryId;
};

void ParseHeader(std::ifstream& pckFile)
{
    // Read header data
    uint32_t dataStartRelative;
    uint32_t unkVal = 0;
    uint32_t bankEntryTableSize, trackEntryTableSize;

    ReadBytesUInt32(dataStartRelative, pckFile);
    ReadBytesUInt32(unkVal, pckFile);
    ReadBytesUInt32(pckCategoryChunkSize, pckFile);

    std::cout << "DataStart (relative): " << dataStartRelative << "\n";
    std::cout << "PCKCategoryChunk Size: " << pckCategoryChunkSize << "\n";

    // Read SubHeader data
    uint32_t checkVal, unkVal2, unkVal3;

    ReadBytesUInt32(checkVal, pckFile);

    if (checkVal == 4)
    {
        isBankPCK = false;
    }
    else
    {
        isBankPCK = true;
    }

    pckFile.seekg(16, std::ios::beg);

    if (isBankPCK)
    {
        ReadBytesUInt32(bankEntryTableSize, pckFile);
        ReadBytesUInt32(unkVal2, pckFile);
        ReadBytesUInt32(unkVal3, pckFile);

        std::cout << "BankEntryTableSize: " << bankEntryTableSize << "\n";
    }
    else
    {
        ReadBytesUInt32(unkVal2, pckFile);
        ReadBytesUInt32(trackEntryTableSize, pckFile);
        ReadBytesUInt32(unkVal3, pckFile);

        std::cout << "TrackEntryTableSize: " << trackEntryTableSize << "\n";
    }
}

void ParsePCKCategoryChunk(std::ifstream& pckFile)
{
    ReadBytesUInt32(pckCategoryCount, pckFile);

    std::cout << "\n";
    std::cout << "PCKCategoryCount: " << pckCategoryCount << "\n";

    //uint32_t currentStringOffset, currentID;
    std::streampos currentChunkPos;
    uint32_t categoryChunkAmountRead = 4;
    PCKCategoryEntry pckCategoryEntry{};
    categoryBuffer = new std::string[pckCategoryCount];

    for (uint32_t i = 0; i < pckCategoryCount; i++)
    {
        ReadBytesUInt32(pckCategoryEntry.offset, pckFile);
        pckCategoryEntry.offset += 28;

        ReadBytesUInt32(pckCategoryEntry.id, pckFile);
        categoryChunkAmountRead += 8;

        currentChunkPos = pckFile.tellg();
        pckFile.seekg(pckCategoryEntry.offset);

        uint8_t categorySize = 0;
        uint8_t b = 0;
        uint32_t pos = pckCategoryEntry.offset;
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

        std::cout << category << " " << pckCategoryEntry.id << "\n";
        categoryBuffer[pckCategoryEntry.id] = category;

        if (i != pckCategoryCount - 1)
        {
            pckFile.seekg(currentChunkPos);
        }
    }

    currentChunkPos = pckFile.tellg();
    uint32_t categoryChunkRemaining = pckCategoryChunkSize - categoryChunkAmountRead;

    if (categoryChunkRemaining != 0)
    {
        currentChunkPos += categoryChunkRemaining;
        pckFile.seekg(currentChunkPos);
        currentChunkPos = pckFile.tellg();
    }

    if (!isBankPCK)
    {
        currentChunkPos += 4;
        pckFile.seekg(currentChunkPos);
    }

    std::cout << "\n";
}

void UnpackFromPCK(std::ifstream& pckFile)
{
    uint32_t trackOrBankCount;
    ReadBytesUInt32(trackOrBankCount, pckFile);
    std::cout << "Track/Bank Count: " << trackOrBankCount << "\n\n";

    std::streampos currentEntryPos;
    std::string categoryName;
    std::string outFile;

    TrackOrBankEntry trackOrBankEntry{};
    int delStatus;
    dirDel = false;

    for (uint32_t i = 0; i < trackOrBankCount; i++)
    {
        ReadBytesUInt32(trackOrBankEntry.id, pckFile);
        ReadBytesUInt32(trackOrBankEntry.unkVal, pckFile);
        ReadBytesUInt32(trackOrBankEntry.size, pckFile);
        ReadBytesUInt32(trackOrBankEntry.offset, pckFile);
        ReadBytesUInt32(trackOrBankEntry.categoryId, pckFile);

        currentEntryPos = pckFile.tellg();

        std::cout << "ID: " << trackOrBankEntry.id << "\n";
        std::cout << "Size: " << trackOrBankEntry.size << "\n";
        std::cout << "Offset: " << trackOrBankEntry.offset << "\n";

        categoryName = categoryBuffer[trackOrBankEntry.categoryId];
        std::cout << "Category: " << categoryName << "\n";

        outFile = extractDir + "\\" + categoryName;
        CreateDirCustom(outFile.c_str(), dirDel);

        if (isBankPCK)
        {
            outFile += "\\" + std::to_string(trackOrBankEntry.id) + ".bnk";
        }
        else
        {
            outFile += "\\" + std::to_string(trackOrBankEntry.id) + ".wem";
        }

        delStatus = remove(outFile.c_str());

        if (delStatus == 0)
        {
            std::cout << "File exists! will be replaced!" << "\n";
        }

        std::ofstream trackFile(outFile, std::ofstream::binary);
        pckFile.seekg(trackOrBankEntry.offset);

        char* buffer = new char[trackOrBankEntry.size];
        pckFile.read(buffer, trackOrBankEntry.size);
        trackFile.write(buffer, trackOrBankEntry.size);
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

        pckFile.seekg(currentEntryPos);
    }
}

int InitiateUnpack(std::ifstream& pckFile, std::string& file)
{
    // Prep before extraction
    std::cout << "PCKFile: " << file <<"\n\n";

    std::string dir = file.substr(0, file.find_last_of("\\"));
    std::string fileName = file.substr(file.find_last_of("\\") + 1);
    extractDir = dir + "\\_" + fileName;

    dirDel = true;
    CreateDirCustom(extractDir.c_str(), dirDel);

    // Read Header data
    char magic[4]{};

    pckFile.read(reinterpret_cast<char*>(&magic), 4);
    if (strncmp(magic, "AKPK", 4) != 0)
    {
        std::cout << "Error: PCK file is invalid!";
        return -1;
    }

    ParseHeader(pckFile);
    
    // Read PCKCategoryChunk data
    ParsePCKCategoryChunk(pckFile);
    
    // Unpack Track or Bank files
    UnpackFromPCK(pckFile);

    std::cout << "\nFinished unpacking pck file!\n";

    return 0;
}