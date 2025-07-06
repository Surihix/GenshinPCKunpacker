#include <iostream>
#include <fstream>
#include "PCKUnpack.h"

int main(int argc, char* argv[])
{
    std::cout << "\n";

    try 
    {
        // Args check
        if (argc < 2)
        {
            std::cout << "Error: Input file not specified!\n";
            return -1;
        }

        std::string file(argv[1]);
        std::ifstream pckFile(file, std::ifstream::binary);

        // Check if file was opened properly
        if (pckFile.fail())
        {
            std::cout << "Error: Failed to open file!";
            return -1;
        }

        // Begin unpack
        InitiateUnpack(pckFile, file);
    }
    catch (std::exception e)
    {
        std::cout << e.what();
    }
}