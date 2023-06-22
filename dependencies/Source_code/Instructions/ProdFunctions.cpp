#include "Display.hpp"
#include <algorithm>

extern const int FONTSET_SIZE;
extern const uint16_t FONTSET_START_ADDRESS;

#pragma region OpcodesProd
void Chip8::OP_NULL()
{
}

void Chip8::OP_00E0()
{
    std::fill(video, video + sizes::VIDEO_HEIGHT * sizes::VIDEO_WIDTH - 1, 0);
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
    // Testing
    
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
    
    byte carry = (registers[Vx] & 0x1u);
    registers[Vx] = registers[Vx] >> 1;
    registers[0xF] = carry;
} // SHR Vx {, Vy}

void Chip8::OP_8xy7()
{
    byte Vx = D_Opd_0x00();
    byte Vy = D_Opd_00x0();
    
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
    byte carry = ((registers[Vx] & 0x80u) >> 7u);
    registers[Vx] = registers[Vx] << 1;
    registers[0xF] = carry;
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
        registers[Vx] = i;
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