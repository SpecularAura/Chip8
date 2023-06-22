#include "Display.hpp"
#include <iostream>
#include "SDL2/SDL.h"
#include "Chip8.hpp"
Display::Display(Options options) : chip8{}, options{options}, chipState{emuState::RUNNING}, sampleRate{44100}, volume{1000}
{}

Color::Color(uint32_t colorEncoded) : 
    r{0},
    g{0},
    b{0},
    a{255}
{
    r = (colorEncoded >> 24u) & 0xFFu;
    g = (colorEncoded >> 16u) & 0xFF;
    b = (colorEncoded >> 8u) & 0xFF;
    a = (colorEncoded >> 0u) & 0xFF;
}

Options::Options(uint32_t bColor, uint32_t fColor, uint32_t IPS, uint32_t toneFreq) 
: 
bgColor{bColor}, 
fgColor{fColor}, 
romFile{""}, 
IPS{IPS}, toneFreq{toneFreq}
{
}

bool Display::InitChip(int scaleFactor, const char *romFile)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        std::cout << "Error initializing SDL: " << SDL_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow("Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              sizes::VIDEO_WIDTH * scaleFactor,
                              sizes::VIDEO_HEIGHT * scaleFactor,
                              SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cout << "Error creating window: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cout << "Error creating renderer: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_AudioSpec want, obtained;
    want.freq     = sampleRate;  // Sampling Rate 44100 Hz
    want.channels = 1;  // Mono Audio
    want.samples  = 2048; // Buffer Size for each channel: Total Buffer Size = samples * channels
    want.format   = AUDIO_S16SYS;
    want.callback = Display::audioCallback;
    want.userdata = static_cast<void*>(this);

    devId = SDL_OpenAudioDevice(
        NULL,
        0,
        &want,
        &obtained,
        0
    );
    if (devId == 0) {
        SDL_Log("Could not get an Audio Device %s\n", SDL_GetError());
        return false;
    }
    if ((want.format != obtained.format) ||
        (want.channels != obtained.channels)) {

        std::cout << "Could not get desired Audio Spec\n";
        return false;
    }

    ClearScreen();
    SDL_RenderPresent(renderer);
    chip8.LoadRom(romFile);
    options.romFile = romFile;
    options.scaleFactor = scaleFactor;
    return true;
}

void Display::audioCallback(void* userdata, Uint8* stream, int len)
{
    Display* object = static_cast<Display*>(userdata);
    object->RenderAudio(stream, len);
}

void Display::RenderAudio(Uint8* stream, int len)
{
    static int32_t sampleCount = 0;
    int16_t* audio_stream = (int16_t*) stream;
    // static bool isHigh = false;
    const int32_t square_wave_period = sampleRate / options.toneFreq;
    const int32_t half_square_wave_period = square_wave_period / 2;
    for(int i = 0; i < len / 2; i++)
    {
        audio_stream[i] = (
            (sampleCount++ / half_square_wave_period) % 2
            ? volume 
            : -volume);
    }
    
}

void Display::RunChip()
{
    while (chipState != QUIT)
    {
        ProcessInput();
        if(chipState == PAUSED || chipState == QUIT){continue;}
        const uint64_t start_frame_time = SDL_GetPerformanceCounter();
        
        // Emulate CHIP8 Instructions for this emulator "frame" (60hz)
        for (uint32_t i = 0; i < options.IPS / 60; i++)
        {
            chip8.Cycle();
        }

        // Get time elapsed after running instructions
        const uint64_t end_frame_time = SDL_GetPerformanceCounter();

        // Delay for approximately 60hz/60fps (16.67ms) or actual time elapsed
        const double time_elapsed = (double)((end_frame_time - start_frame_time) * 1000) / SDL_GetPerformanceFrequency();

        SDL_Delay(16.67f > time_elapsed ? 16.67f - time_elapsed : 0);
        // SDL_Delay(100);

        if(chip8.screenUpdate)
        {
            Render();
            chip8.screenUpdate = false;
        }

        bool isTimer0 = chip8.UpdateTimers();
        if(isTimer0)
        {
            SDL_PauseAudioDevice(devId, 1); // Pause Sound
        }
        else
        {
            SDL_PauseAudioDevice(devId, 0); // Play Sound
        }

    }
}

void Display::ProcessInput()
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                // Exit window; End program
                chipState = QUIT; // Will exit main emulator loop
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        // Escape key; Exit window & End program
                        chipState = QUIT;
                        break;
                        
                    case SDLK_SPACE:
                        // Space bar
                        if (chipState == RUNNING) {
                            chipState = PAUSED;  // Pause
                            puts("==== PAUSED ====");
                        } else {
                            chipState = RUNNING; // Resume
                        }
                        break;

                    case SDLK_EQUALS:
                        // '=': Reset CHIP8 machine for the current ROM
                        chip8.Reset(true, options.romFile);
                        break;

                    // Map qwerty keys to CHIP8 keypad
                    case SDLK_1: chip8.keypad[0x1] = 1; break;
                    case SDLK_2: chip8.keypad[0x2] = 1; break;
                    case SDLK_3: chip8.keypad[0x3] = 1; break;
                    case SDLK_4: chip8.keypad[0xC] = 1; break;

                    case SDLK_q: chip8.keypad[0x4] = 1; break;
                    case SDLK_w: chip8.keypad[0x5] = 1; break;
                    case SDLK_e: chip8.keypad[0x6] = 1; break;
                    case SDLK_r: chip8.keypad[0xD] = 1; break;

                    case SDLK_a: chip8.keypad[0x7] = 1; break;
                    case SDLK_s: chip8.keypad[0x8] = 1; break;
                    case SDLK_d: chip8.keypad[0x9] = 1; break;
                    case SDLK_f: chip8.keypad[0xE] = 1; break;

                    case SDLK_z: chip8.keypad[0xA] = 1; break;
                    case SDLK_x: chip8.keypad[0x0] = 1; break;
                    case SDLK_c: chip8.keypad[0xB] = 1; break;
                    case SDLK_v: chip8.keypad[0xF] = 1; break;

                    default: break;
                        
                }
                break; 

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    // Map qwerty keys to CHIP8 keypad
                    case SDLK_1: chip8.keypad[0x1] = 0; break;
                    case SDLK_2: chip8.keypad[0x2] = 0; break;
                    case SDLK_3: chip8.keypad[0x3] = 0; break;
                    case SDLK_4: chip8.keypad[0xC] = 0; break;

                    case SDLK_q: chip8.keypad[0x4] = 0; break;
                    case SDLK_w: chip8.keypad[0x5] = 0; break;
                    case SDLK_e: chip8.keypad[0x6] = 0; break;
                    case SDLK_r: chip8.keypad[0xD] = 0; break;

                    case SDLK_a: chip8.keypad[0x7] = 0; break;
                    case SDLK_s: chip8.keypad[0x8] = 0; break;
                    case SDLK_d: chip8.keypad[0x9] = 0; break;
                    case SDLK_f: chip8.keypad[0xE] = 0; break;

                    case SDLK_z: chip8.keypad[0xA] = 0; break;
                    case SDLK_x: chip8.keypad[0x0] = 0; break;
                    case SDLK_c: chip8.keypad[0xB] = 0; break;
                    case SDLK_v: chip8.keypad[0xF] = 0; break;

                    default: break;
                }
                break;

            default:
                break;
        }
    }
}

void Display::Render()
{
    ClearScreen();
    SDL_Rect rect;
    rect.w = options.scaleFactor;
    rect.h = options.scaleFactor;

    for(int i = 0; i < (sizes::VIDEO_WIDTH * sizes::VIDEO_HEIGHT); i++)
    {
        rect.x = (i % sizes::VIDEO_WIDTH) * options.scaleFactor;
        rect.y = (i / sizes::VIDEO_WIDTH) * options.scaleFactor;

        if(chip8.video[i])
        {
            SDL_SetRenderDrawColor(renderer, options.fgColor.r, options.fgColor.g, options.fgColor.b, options.fgColor.a);
            SDL_RenderFillRect(renderer, &rect);
            SDL_SetRenderDrawColor(renderer, options.bgColor.r, options.bgColor.g, options.bgColor.b, options.bgColor.a);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}

void Display::ClearScreen()
{
    SDL_SetRenderDrawColor(renderer, options.bgColor.r, options.bgColor.g, options.bgColor.b, options.bgColor.a);
    SDL_RenderClear(renderer);
}


Display::~Display()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_CloseAudioDevice(devId);
    window = nullptr;
    renderer = nullptr;
    SDL_Quit();
}