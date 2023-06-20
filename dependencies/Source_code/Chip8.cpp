#include "Chip8.hpp"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <functional>
#include <random>
const int FONTSET_SIZE = 80;
const uint16_t FONTSET_START_ADDRESS = 0x000;
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

// int Chip8::LoadRom(const char *filename)
// {
//     std::ifstream file(filename, std::ios::binary | std::ios::ate);
//     std::streampos size;
//     if (file.is_open())
//     {
//         size = file.tellg();
//         char *buffer = new char[size];
//         file.seekg(0, std::ios::beg);
//         file.read(buffer, size);
//         file.close();

//         std::ios init(NULL);
//         init.copyfmt(std::cout);
//         for (long i = 0; i < size; ++i)
//         {
//             memory[START_ADDRESS + i] = buffer[i];

//             std::cout << std::hex << START_ADDRESS + i << ": ";
//             std::cout << std::hex << std::setw(2) << std::setfill('0')
//                       << ((+buffer[i]) & 0xFFu);
//             std::cout << "\n";
//         }
//         std::cout.copyfmt(init);
//         delete[] buffer;
//     }
//     return size;
// }

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

#ifdef DEBUG
#pragma region Opcodes
void Chip8::OP_NULL()
{
}

void Chip8::OP_00E0()
{
    std::cout << "Cleared Video Buffer\n";
    std::fill(video, video + sizes::VIDEO_HEIGHT * sizes::VIDEO_WIDTH - 1, 0);
    screenUpdate = true;
} // CLS

void Chip8::OP_00EE()
{
    std::cout << "PC changed from " << std::hex << PC << "to " << stack[SP - 1] << "\n";
    PC = stack[SP - 1];
    --SP;

} // RET

void Chip8::OP_1nnn()
{
    doubleByte address = D_Opd_0xxx();
    //     0010 xxxx xxxx xxxx
    //   & 0000 1111 1111 1111
    //    ____________________
    //          xxxx xxxx xxxx -> address
    std::ios init(NULL);
    init.copyfmt(std::cout);
    std::cout << "PC changed from " << std::hex << PC << " to " << address << "\n";
    std::cout.copyfmt(init);

    PC = address;
} // JP nnn

void Chip8::OP_2nnn()
{
    doubleByte address = D_Opd_0xxx();
    //     0010 xxxx xxxx xxxx
    //   & 0000 1111 1111 1111
    //    ____________________
    //          xxxx xxxx xxxx -> address

    ++SP;
    stack[SP - 1] = PC;

    std::ios init(NULL);
    init.copyfmt(std::cout);
    std::cout << "CALL " << std::hex << address << "\n";
    std::cout.copyfmt(init);

    PC = address;
} // CALL nnn

void Chip8::OP_3xkk()
{
    byte Vx = D_Opd_0x00();
    //     0010 xxxx kkkk kkkk
    //   & 0000 1111 0000 0000
    //    ____________________
    //          xxxx 0000 0000 (0x0n00)
    //          (reg)

    // xxxx 0000 0000 >> 8 -> xxxx
    byte kk = D_Opd_00xx();
    // Testing
    std::cout << "SE V";
    printByte(Vx);
    std::cout << ", ";
    printByte(kk);
    std::cout << "\n";

    if (registers[Vx] == kk)
    {
        PC += 2;
    }
} // SE Vx, kk

void Chip8::OP_4xkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();
    std::cout << "SNE V";
    printByte(Vx);
    std::cout << ", ";
    printByte(kk);
    std::cout << "\n";

    if (registers[Vx] != kk)
    {
        PC += 2;
    }
} // SNE Vx, kk

void Chip8::OP_5xy0()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "SE V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    if (registers[Vx] == registers[Vy])
    {
        PC += 2;
    }
} // SE Vx, Vy

void Chip8::OP_6xkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();
    std::cout << "LD V";
    printByte(Vx);
    std::cout << ", ";
    printByte(kk);
    std::cout << "\n";

    registers[Vx] = kk;
} // LD Vx, kk

void Chip8::OP_7xkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();
    std::cout << "ADD V";
    printByte(Vx);
    std::cout << ", ";
    printByte(kk);
    std::cout << "\n";

    registers[Vx] = registers[Vx] + kk;
} // ADD Vx, kk

void Chip8::OP_8xy0()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "LD V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    registers[Vx] = registers[Vy];
} // LD Vx, Vy

void Chip8::OP_8xy1()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "OR V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    registers[Vx] = registers[Vx] | registers[Vy];
} // OR Vx, Vy

void Chip8::OP_8xy2()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "AND V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    registers[Vx] = registers[Vx] & registers[Vy];
} // AND Vx, Vy

void Chip8::OP_8xy3()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "XOR V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    registers[Vx] = registers[Vx] ^ registers[Vy];
} // XOR Vx, Vy

void Chip8::OP_8xy4()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "ADD V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    doubleByte sum = (doubleByte)registers[Vx] + (doubleByte)registers[Vy];
    registers[Vx] = static_cast<byte>(sum & 0xFFu);
    if (sum > 255u)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }
} // ADD Vx, Vy

void Chip8::OP_8xy5()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "SUB V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";
    
    if (registers[Vx] > registers[Vy])
    {
        registers[Vx] = registers[Vx] - registers[Vy];
        registers[0xF] = 1;
    }
    else
    {
        registers[Vx] = registers[Vx] - registers[Vy];
        registers[0xF] = 0;
    }
} // SUB Vx, Vy

void Chip8::OP_8xy6()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "SHR V";
    printByte(Vx);
    std::cout << "\n";

    byte carry = (registers[Vx] & 0x1u);
    registers[Vx] = registers[Vx] >> 1;
    registers[0xF] = carry;
} // SHR Vx {, Vy}

void Chip8::OP_8xy7()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "SUBN V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    registers[Vx] = registers[Vy] - registers[Vx];
    if (registers[Vy] > registers[Vx])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

} // SUBN Vx, Vy

void Chip8::OP_8xyE()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "SHL V";
    printByte(Vx);
    std::cout << "\n";

    byte carry = ((registers[Vx] & 0x80u) >> 7u);
    registers[Vx] = registers[Vx] << 1;
    registers[0xF] = carry;
} // SHL Vx {, Vy}

void Chip8::OP_9xy0()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    std::cout << "SNE V";
    printByte(Vx);
    std::cout << ", V";
    printByte(Vy);
    std::cout << "\n";

    if (registers[Vx] != registers[Vy])
    {
        PC += 2;
    }
} // SNE Vx, Vy

void Chip8::OP_Annn()
{
    doubleByte address = D_Opd_0xxx();
    std::ios init(NULL);
    init.copyfmt(std::cout);
    std::cout << "Index Register loaded with: " << std::hex << address << "\n";
    std::cout.copyfmt(init);

    Index = address;
} // LD I, nnn

void Chip8::OP_Bnnn()
{
    doubleByte address = D_Opd_0xxx();
    PC = address + registers[0];

    std::cout << "Jumped to: " << PC << "\n";
} // JP V0, nnn

void Chip8::OP_Cxkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();

    registers[Vx] = rand(mt) & kk;

    std::cout << "V";
    printByte(Vx);
    std::cout << ": ";
    printByte(registers[Vx]);
    std::cout << "\n";
} // RND Vx, kk

void Chip8::OP_Dxyn()
{
    using namespace sizes;
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    byte height = D_Opd_000x();

    Vx = registers[Vx] % VIDEO_WIDTH;
    Vy = registers[Vy] % VIDEO_HEIGHT;
    std::ios init(NULL);
    init.copyfmt(std::cout);
    std::cout << std::hex << "Drawing Sprite at " << Index;
    std::cout << std::dec << " Starting From (" << (+Vx & 0xFFu) << ", " << (+Vy & 0xFFu) << ")";
    std::cout << "\n";
    registers[0xF] = 0;
    for (byte row = 0; row < height; ++row)
    {
        byte spriteByte = memory[Index + row];
        byte mask = 0x80; // 1000 0000
        for (byte col = 0; col < 8; ++col)
        {
            byte spritePixel = spriteByte & (0x80u >> col);
            if (spritePixel)
            {
                uint32_t *screenPixel = &video[(Vy + row) * VIDEO_WIDTH + (Vx + col)];
                if (*screenPixel)
                {
                    registers[0xF] = 1;
                }
                *screenPixel = *screenPixel ^ 0xFFFFFFFFu;
            }
            mask = mask >> 1;
            if(Vx + col + 1 >= VIDEO_WIDTH){break;}
        }
        if(Vy + row + 1 >= VIDEO_HEIGHT){break;}
    }
    std::cout.copyfmt(init);
    screenUpdate = true;
} // DRW Vx, Vy, nibble

void Chip8::OP_Ex9E()
{
    byte Vx = D_Opd_0x00();
    std::cout << "SKP V";
    printByte(Vx);
    std::cout << "\n";
    if (keypad[registers[Vx]])
    {
        PC += 2;
    }
} // SKP Vx

void Chip8::OP_ExA1()
{
    byte Vx = D_Opd_0x00();
    std::cout << "SKNP V";
    printByte(Vx);
    std::cout << "\n";
    if (!keypad[registers[Vx]])
    {
        PC += 2;
    }
} // SKNP Vx

void Chip8::OP_Fx07()
{
    byte Vx = D_Opd_0x00();
    std::cout << "LD V";
    printByte(Vx);
    printByte(delayTimer);
    std::cout << "\n";

    registers[Vx] = delayTimer;
} // LD Vx, DT

void Chip8::OP_Fx0A()
{
    byte Vx = D_Opd_0x00();
    byte i = 0;
    for (i = 0; i < sizes::numKeys; i++)
    {
        if (keypad[i])
        {
            break;
        }
    }
    if (i < sizes::numKeys)
    {
        registers[Vx] = i;
        std::cout << "LD V";
        printByte(Vx);
        std::cout << i;
        std::cout << "\n";
    }
    else
    {
        PC = PC - 2;
    }
} // LD Vx, K

void Chip8::OP_Fx15()
{
    byte Vx = D_Opd_0x00();
    std::cout << "LD DT";
    std::cout << ", V";
    printByte(Vx);
    std::cout << "\n";

    delayTimer = registers[Vx];
} // LD DT, Vx

void Chip8::OP_Fx18()
{
    byte Vx = D_Opd_0x00();
    std::cout << "LD ST";
    std::cout << ", V";
    printByte(Vx);
    std::cout << "\n";

    soundTimer = registers[Vx];
} // LD ST, Vx

void Chip8::OP_Fx1E()
{
    byte Vx = D_Opd_0x00();
    Index = Index + (doubleByte)registers[Vx];

    std::cout << "Undocumented"
              << "\n";
} // ADD I, Vx

void Chip8::OP_Fx29()
{
    byte Vx = D_Opd_0x00();

    Index = FONTSET_START_ADDRESS + (5 * registers[Vx]);
    std::cout << "Undocumented"
              << "\n";
} // LD F, Vx

void Chip8::OP_Fx33()
{
    byte Vx = D_Opd_0x00();
    doubleByte value = registers[Vx];
    memory[Index + 2] = value % 10;
    value = value / 10;
    memory[Index + 1] = value % 10;
    value = value / 10;
    memory[Index] = value % 10;
    std::cout << "Undocumented"
              << "\n";
} // LD B, Vx

void Chip8::OP_Fx55()
{
    byte Vx = D_Opd_0x00();
    for (byte i = 0; i <= Vx; i++)
    {
        memory[Index + i] = registers[i];
    }
    std::cout << "Undocumented"
              << "\n";

} // LD [I], Vx

void Chip8::OP_Fx65()
{
    byte Vx = D_Opd_0x00();
    for (byte i = 0; i <= Vx; i++)
    {
        registers[i] = memory[Index + i];
    }
    std::cout << "Undocumented"
              << "\n";

} // LD Vx, [I]

#pragma endregion Opcodes

#else

#pragma region OpcodesProd
void Chip8::OP_NULL()
{
}

void Chip8::OP_00E0()
{
    screenUpdate = true;
} // CLS

void Chip8::OP_00EE()
{
    PC = stack[SP - 1];
    --SP;

} // RET

void Chip8::OP_1nnn()
{
    doubleByte address = D_Opd_0xxx();
    //     0010 xxxx xxxx xxxx
    //   & 0000 1111 1111 1111
    //    ____________________
    //          xxxx xxxx xxxx -> address
    PC = address;
} // JP nnn

void Chip8::OP_2nnn()
{
    doubleByte address = D_Opd_0xxx();
    //     0010 xxxx xxxx xxxx
    //   & 0000 1111 1111 1111
    //    ____________________
    //          xxxx xxxx xxxx -> address

    ++SP;
    stack[SP - 1] = PC;

    PC = address;
} // CALL nnn

void Chip8::OP_3xkk()
{
    byte Vx = D_Opd_0x00();
    //     0010 xxxx kkkk kkkk
    //   & 0000 1111 0000 0000
    //    ____________________
    //          xxxx 0000 0000 (0x0n00)
    //          (reg)

    // xxxx 0000 0000 >> 8 -> xxxx
    byte kk = D_Opd_00xx();

    if (registers[Vx] == kk)
    {
        PC += 2;
    }
} // SE Vx, kk

void Chip8::OP_4xkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();

    if (registers[Vx] != kk)
    {
        PC += 2;
    }
} // SNE Vx, kk

void Chip8::OP_5xy0()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    if (registers[Vx] == registers[Vy])
    {
        PC += 2;
    }
} // SE Vx, Vy

void Chip8::OP_6xkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();

    registers[Vx] = kk;
} // LD Vx, kk

void Chip8::OP_7xkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();

    registers[Vx] = registers[Vx] + kk;
} // ADD Vx, kk

void Chip8::OP_8xy0()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    registers[Vx] = registers[Vy];
} // LD Vx, Vy

void Chip8::OP_8xy1()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    registers[Vx] = registers[Vx] | registers[Vy];
} // OR Vx, Vy

void Chip8::OP_8xy2()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    registers[Vx] = registers[Vx] & registers[Vy];
} // AND Vx, Vy

void Chip8::OP_8xy3()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    registers[Vx] = registers[Vx] ^ registers[Vy];
} // XOR Vx, Vy

void Chip8::OP_8xy4()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    doubleByte sum = (doubleByte)registers[Vx] + (doubleByte)registers[Vy];
    if (sum > 255u)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = static_cast<byte>(sum & 0xFFu);
} // ADD Vx, Vy

void Chip8::OP_8xy5()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    if (registers[Vx] > registers[Vy])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vx] - registers[Vy];
} // SUB Vx, Vy

void Chip8::OP_8xy6()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] = registers[Vx] >> 1;
} // SHR Vx {, Vy}

void Chip8::OP_8xy7()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    if (registers[Vy] > registers[Vx])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
} // SUBN Vx, Vy

void Chip8::OP_8xyE()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    registers[0xF] = ((registers[Vx] & 0x80u) >> 7u);
    registers[Vx] = registers[Vx] << 1;
} // SHL Vx {, Vy}

void Chip8::OP_9xy0()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();

    if (registers[Vx] != registers[Vy])
    {
        PC += 2;
    }
} // SNE Vx, Vy

void Chip8::OP_Annn()
{
    doubleByte address = D_Opd_0xxx();

    Index = address;
} // LD I, nnn

void Chip8::OP_Bnnn()
{
    doubleByte address = D_Opd_0xxx();
    PC = address + registers[0];
} // JP V0, nnn

void Chip8::OP_Cxkk()
{
    byte Vx = D_Opd_0x00();
    byte kk = D_Opd_00xx();

    registers[Vx] = rand(mt) & kk;
} // RND Vx, kk

void Chip8::OP_Dxyn()
{
    using namespace sizes;
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    byte height = D_Opd_000x();

    Vx = registers[Vx] % VIDEO_WIDTH;
    Vy = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;
    for (byte row = 0; row < height; ++row)
    {
        byte spriteByte = memory[Index + row];
        byte mask = 0x80; // 1000 0000
        for (byte col = 0; col < 8; ++col)
        {
            byte spritePixel = spriteByte & (0x80 >> col);
            if (spritePixel)
            {
                uint32_t *screenPixel = &video[(Vy + row) * VIDEO_WIDTH + (Vx + col)];
                if (*screenPixel)
                {
                    registers[0xF] = 1;
                }
                *screenPixel = *screenPixel ^ 0xFFFFFFFFu;
            }
            mask = mask >> 1;
            if(Vx + col + 1 >= VIDEO_WIDTH){break;}
        }
        if(Vy + row + 1 >= VIDEO_HEIGHT){break;}
    }
    screenUpdate = true;
} // DRW Vx, Vy, nibble

void Chip8::OP_Ex9E()
{
    byte Vx = D_Opd_0x00();
    if (keypad[registers[Vx]])
    {
        PC += 2;
    }
} // SKP Vx

void Chip8::OP_ExA1()
{
    byte Vx = D_Opd_0x00();
    if (!keypad[registers[Vx]])
    {
        PC += 2;
    }
} // SKNP Vx

void Chip8::OP_Fx07()
{
    byte Vx = D_Opd_0x00();

    registers[Vx] = delayTimer;
} // LD Vx, DT

void Chip8::OP_Fx0A()
{
    byte Vx = D_Opd_0x00();
    byte i = 0;
    for (i = 0; i < sizes::numKeys; i++)
    {
        if (keypad[i])
        {
            break;
        }
    }
    if (i < sizes::numKeys)
    {
    }
    else
    {
        PC = PC - 2;
    }
} // LD Vx, K

void Chip8::OP_Fx15()
{
    byte Vx = D_Opd_0x00();

    delayTimer = registers[Vx];
} // LD DT, Vx

void Chip8::OP_Fx18()
{
    byte Vx = D_Opd_0x00();

    soundTimer = registers[Vx];
} // LD ST, Vx

void Chip8::OP_Fx1E()
{
    byte Vx = D_Opd_0x00();
    Index = Index + (doubleByte)registers[Vx];

} // ADD I, Vx

void Chip8::OP_Fx29()
{
    byte Vx = D_Opd_0x00();

    Index = FONTSET_START_ADDRESS + (5 * registers[Vx]);
} // LD F, Vx

void Chip8::OP_Fx33()
{
    byte Vx = D_Opd_0x00();
    doubleByte value = registers[Vx];
    memory[Index + 2] = value % 10;
    value = value / 10;
    memory[Index + 1] = value % 10;
    value = value / 10;
    memory[Index] = value % 10;
} // LD B, Vx

void Chip8::OP_Fx55()
{
    byte Vx = D_Opd_0x00();
    for (byte i = 0; i <= Vx; i++)
    {
        memory[Index + i] = registers[i];
    }
} // LD [I], Vx

void Chip8::OP_Fx65()
{
    byte Vx = D_Opd_0x00();
    for (byte i = 0; i <= Vx; i++)
    {
        registers[i] = memory[Index + i];
    }
} // LD Vx, [I]

#pragma endregion OpcodesProd
#endif



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
