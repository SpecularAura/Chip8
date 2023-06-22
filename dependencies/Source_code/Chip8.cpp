#include "Chip8.hpp"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <functional>
#include <random>
extern const int FONTSET_SIZE = 80;
extern const uint16_t FONTSET_START_ADDRESS = 0x000;
using byte = uint8_t;
using doubleByte = uint16_t;
Chip8::Chip8() : Index{0x000}, PC{START_ADDRESS}, SP{0}, delayTimer{0}, soundTimer{0}, IP{0x000},  screenUpdate{true}
{
    memory = new byte[sizes::memSize];
    video = new uint32_t[64 * 32];
    initFuncPointerTable();
    Reset();
}

void Chip8::Reset(bool shouldLoadRom, std::string filename)
{
    SP = 0;
    delayTimer = 0;
    soundTimer = 0;
    IP = 0x000;
    PC = START_ADDRESS;
    Index = 0x000;
    screenUpdate = true;
    std::fill(std::begin(registers), std::end(registers), 0);
    std::fill(std::begin(stack), std::end(stack), 0);
    std::fill(std::begin(keypad), std::end(keypad), 0);
    std::fill(memory, memory + sizes::memSize - 1, 0);
    std::fill(video, video + (sizes::VIDEO_HEIGHT) * (sizes::VIDEO_WIDTH) - 1, 0);

    uint8_t fontset[FONTSET_SIZE] =
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

    for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
    {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Random Number
    std::random_device rd{};
    // Create seed_seq with high-res clock and 7 random numbers from std::random_device
    std::seed_seq ss{
        static_cast<std::seed_seq::result_type>(std::chrono::steady_clock::now().time_since_epoch().count()),
        rd(), rd(), rd(), rd(), rd(), rd(), rd()};

    mt = std::mt19937{ss};
    rand = std::uniform_int_distribution<uint8_t>{0, 255U};

    if (shouldLoadRom)
    {
        LoadRom(filename.c_str());
    }
}

void Chip8::Cycle()
{
    IP = (memory[PC] << 8u) | memory[PC + 1];
    PC += 2;

    #ifdef DEBUG
    printState();
    #endif
    (this->*(table[D_Opd_x000()]))();
}

bool Chip8::UpdateTimers()
{
    if (delayTimer > 0)
    {
        --delayTimer;
    }

    if (soundTimer > 0)
    {
        --soundTimer;
        return false;
    }
    return true;
}

void Chip8::initFuncPointerTable()
{
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    for (int i = 0; i < 0xE + 1; i++)
    {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }
    for (int i = 0; i < 0x65 + 1; i++)
    {
        tableF[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;
}

Chip8::~Chip8()
{
    delete[] memory;
    delete[] video;
    PC = START_ADDRESS;
}

int Chip8::LoadRom(const char *filename)
{
    int i = 0;
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open())
    {
        char x;
        std::ios init(NULL);
        init.copyfmt(std::cout);
        while (file)
        {
            file.read(&x, 1);
            memory[START_ADDRESS + i] = x;
            std::cout << std::hex << START_ADDRESS + i << ": ";
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << ((+x) & 0xFFu);
			   std::cout << "\n";
            i++;
        }
        std::cout.copyfmt(init);
        file.close();
    }
    return i;
}

#pragma region helpers
byte Chip8::D_Opd_00x0()
{
    return static_cast<byte>((IP & static_cast<doubleByte>(0x00F0)) >> 4u);
}

byte Chip8::D_Opd_000x()
{
    return static_cast<byte>((IP & static_cast<doubleByte>(0x000F)));
}

byte Chip8::D_Opd_0x00()
{
    return static_cast<byte>((IP & static_cast<doubleByte>(0x0F00)) >> 8u);
}

byte Chip8::D_Opd_x000()
{
    return static_cast<byte>((IP & static_cast<doubleByte>(0xF000)) >> 12u);
}

byte Chip8::D_Opd_xx00()
{
    return static_cast<byte>((IP & static_cast<doubleByte>(0xFF00)) >> 8u);
}

byte Chip8::D_Opd_00xx()
{
    return static_cast<byte>(IP & static_cast<doubleByte>(0x00FF));
}

byte Chip8::D_Opd_0xx0()
{
    return static_cast<byte>((IP & static_cast<doubleByte>(0x0FF0)) >> 4u);
}

doubleByte Chip8::D_Opd_0xxx()
{
    return (IP & static_cast<doubleByte>(0x0FFF));
}

void Chip8::printState()
{
    std::ios init(NULL);
    init.copyfmt(std::cout);
    std::cout << "PC: " << std::setw(3) << std::setfill('0') << std::hex << PC;
    std::cout.copyfmt(init);
    std::cout << " IP: " << std::setw(4) << std::setfill('0') << std::hex << IP;
    std::cout.copyfmt(init);
    std::cout << " Address: " << std::setw(3) << std::setfill('0') << std::hex << Index;
    std::cout.copyfmt(init);
    std::cout << " VF: " << std::hex << ((+registers[0xF]) & 0xFu);
    std::cout << " Desc: ";
    std::cout.copyfmt(init);
}

void Chip8::printByte(byte x)
{
    std::ios init(NULL);
    init.copyfmt(std::cout);
    std::cout << std::hex << std::setw(2) << std::setfill('0') << ((+x) & 0xFFu);
    std::cout.copyfmt(init);
}
#pragma endregion helpers

void Chip8::Table0()
{
    // Alternativ way to call the function
    // std::invoke(table0[D_Opd_000x()], this);
    (this->*(table0[D_Opd_000x()]))();
}

void Chip8::Table8()
{
    (this->*(table8[D_Opd_000x()]))();
}

void Chip8::TableE()
{
    (this->*(tableE[D_Opd_000x()]))();
}

void Chip8::TableF()
{
    (this->*(tableF[D_Opd_00xx()]))();
}
