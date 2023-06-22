#ifndef DISPLAY_HPP
#define DISPLAY_HPP
#include <string>
#include "SDL2/SDL.h"
#include "Chip8.hpp"
class Color
{
    public:
    Uint8 r, g, b, a;
    Color(uint32_t colorEncoded);
};
class Options
{
public:
    Color bgColor, fgColor;
    std::string romFile;
    int scaleFactor;
    uint32_t IPS;
    uint32_t toneFreq;
    Options(uint32_t bgColor = 0x000000ff, uint32_t fgColor = 0xffffffff, uint32_t IPS = 500, 
            uint32_t toneFreq = 440);
};
class Display
{
    enum emuState{
        QUIT,
        PAUSED,
        RUNNING
    };
    Chip8 chip8;
    Options options;
    emuState chipState; 
    SDL_Window *window;
    SDL_Renderer *renderer;
    uint32_t sampleRate;
    int16_t volume;
    SDL_AudioDeviceID devId;

    static void audioCallback(void* userdata, Uint8* stream, int len);
    void RenderAudio(Uint8* stream, int len);
    void ProcessInput();
    void Render();
    void ClearScreen();
public:
    Display(Options options);

    bool InitChip(int scaleFactor, const char *romFile);
    void RunChip();
    ~Display();
};
#endif