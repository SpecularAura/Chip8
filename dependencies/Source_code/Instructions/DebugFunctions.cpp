#include "Display.hpp"
#include <algorithm>
#include <iostream>

extern const int FONTSET_SIZE;
extern const uint16_t FONTSET_START_ADDRESS;

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