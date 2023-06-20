#include <fstream>
#include <iostream>
#include "Chip8.hpp"
int main()
{
    Chip8 chip8;
	int size = chip8.LoadRom("Programs/IBMLogo.ch8");
	std::cout << size << "\n";
	for(int i = 0; i < size; i++)
	{
		chip8.Cycle();
	}
	for(int i = 0; i < sizes::VIDEO_HEIGHT; i++)
    {
        for(int j = 0; j < sizes::VIDEO_WIDTH; j++)
        {
			uint32_t pixel = chip8.video[i * sizes::VIDEO_WIDTH + j];
            if(pixel)
			{
				std::cout << "**";
			}
			else
			{
				std::cout << "  ";
			}
        }
		std::cout << "\n";
    }
}