#include "Display.hpp"

int main(int argc, char** argv) 
{
    if(argv < 2)
    {
        std::cout << "Please Enter a Rom File" << "\n";
        return -1;
    }
    std::string romFile{args[1]};
    Options options;
    // options.toneFreq = 440;
    Display display{options};
    display.InitChip(20, romFile.c_str()); // InitChip(scaleFactor, romFile)
    display.RunChip();

}
