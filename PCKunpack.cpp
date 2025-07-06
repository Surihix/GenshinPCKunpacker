#include <fstream>
#include <iostream>
#include "ReadByteFunctions.h"
#include "SharedFunctions.h"
#include <string>

std::string extractDir;
uint32_t pckCategoryChunkSize = 0;
bool isBankPCK;
uint32_t pckCategoryCount;

typedef struct PCKCategoryEntry
{
    uint32_t offset;
    uint32_t id;
};

typedef struct BankEntry
{
    uint32_t bankId;
    uint32_t unkVal;
    uint32_t bankSize;
    uint32_t bankOffset;
    uint32_t reserved;
};

typedef struct TrackEntry
{
    uint32_t trackId;
    uint32_t unkVal;
    uint32_t trackSize;
    uint32_t trackOffset;
    uint32_t reserved;
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
    uint32_t trackCount, bankCount;
    std::streampos currentEntryPos;
    std::string outFile;

    if (isBankPCK)
    {
        ReadBytesUInt32(bankCount, pckFile);
        std::cout << "BankCount: " << bankCount << "\n\n";

        BankEntry bankEntry{};

        for (uint32_t i = 0; i < bankCount; i++)
        {
            ReadBytesUInt32(bankEntry.bankId, pckFile);
            ReadBytesUInt32(bankEntry.unkVal, pckFile);
            ReadBytesUInt32(bankEntry.bankSize, pckFile);
            ReadBytesUInt32(bankEntry.bankOffset, pckFile);
            ReadBytesUInt32(bankEntry.reserved, pckFile);

            currentEntryPos = pckFile.tellg();

            std::cout << "BankId: " << bankEntry.bankId << "\n";
            std::cout << "Size: " << bankEntry.bankSize << "\n";
            std::cout << "Offset: " << bankEntry.bankOffset << "\n";

            pckFile.seekg(bankEntry.bankOffset);
            outFile = extractDir + "\\" + std::to_string(bankEntry.bankId) + ".bnk";
            std::cout << "Bank: " << outFile << "\n\n";

            pckFile.seekg(currentEntryPos);
        }
    }
    else
    {
        ReadBytesUInt32(trackCount, pckFile);
        std::cout << "TrackCount: " << trackCount << "\n\n";

        TrackEntry trackEntry{};

        for (uint32_t i = 0; i < trackCount; i++)
        {
            ReadBytesUInt32(trackEntry.trackId, pckFile);
            ReadBytesUInt32(trackEntry.unkVal, pckFile);
            ReadBytesUInt32(trackEntry.trackSize, pckFile);
            ReadBytesUInt32(trackEntry.trackOffset, pckFile);
            ReadBytesUInt32(trackEntry.reserved, pckFile);

            currentEntryPos = pckFile.tellg();

            std::cout << "TrackId: " << trackEntry.trackId << "\n";
            std::cout << "Size: " << trackEntry.trackSize << "\n";
            std::cout << "Offset: " << trackEntry.trackOffset << "\n";

            pckFile.seekg(trackEntry.trackOffset);
            outFile = extractDir + "\\" + std::to_string(trackEntry.trackId) + ".wem";
            std::cout << "Track: " << outFile << "\n\n";

            pckFile.seekg(currentEntryPos);
        }
    }
}

int InitiateUnpack(std::ifstream& pckFile, std::string& file)
{
    // Prep before extraction
    std::cout << "PCKFile: " << file <<"\n\n";

    std::string dir = file.substr(0, file.find_last_of("\\"));
    std::string fileName = file.substr(file.find_last_of("\\") + 1);
    extractDir = dir + "\\_" + fileName;

    CreateDir(extractDir.c_str());

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

    return 0;
}