#ifndef CHIP8_HPP
#define CHIP8_HPP
#include <bitset>
#include <cstdio>
#include <random>
#include <functional>
#include <string>
namespace sizes {
    constexpr int numRegisters{16};
    constexpr int memSize{4096};
    constexpr int stackLevels{16};
    constexpr int numKeys{16};
    constexpr int VIDEO_HEIGHT{32};
    constexpr int VIDEO_WIDTH{64};
}

class Chip8 {
    using byte = uint8_t;
    using doubleByte = uint16_t;
    const doubleByte START_ADDRESS = 0x200;
    doubleByte Index; // Index Register
    doubleByte PC; // Program Counter
    byte SP; // Stack Pointer
    byte delayTimer;
    byte soundTimer;
    doubleByte IP; // Instruction Pointer - Opcode
    doubleByte stack[sizes::stackLevels];
    byte registers[sizes::numRegisters]; // 16 8 bit registers - Stack Allocated
    byte* memory; // Addressable locations 4096 - Heap Allocated

    public:
    byte screenUpdate;
    byte keypad[sizes::numKeys];
    uint32_t* video;

    private:
    std::mt19937 mt;
    std::uniform_int_distribution<uint8_t> rand;

    void OP_NULL();
    void OP_00E0(); // CLS
    void OP_00EE(); // RET
    void OP_1nnn(); // JP nnn
    void OP_2nnn(); // CALL nnn
    void OP_3xkk(); // SE Vx, kk
    void OP_4xkk(); // SNE Vx, kk
    void OP_5xy0(); // SE Vx, Vy
    void OP_6xkk(); // LD Vx, kk
    void OP_7xkk(); // ADD Vx, kk
    void OP_8xy0(); // LD Vx, Vy
    void OP_8xy1(); // OR Vx, Vy
    void OP_8xy2(); // AND Vx, Vy
    void OP_8xy3(); // XOR Vx, Vy
    void OP_8xy4(); // ADD Vx, Vy
    void OP_8xy5(); // SUB Vx, Vy
    void OP_8xy6(); // SHR Vx {, Vy}
    void OP_8xy7(); // SUBN Vx, Vy
    void OP_8xyE(); // SHL Vx {, Vy}
    void OP_9xy0(); // SNE Vx, Vy
    void OP_Annn(); // LD I, nnn
    void OP_Bnnn(); // JP V0, nnn
    void OP_Cxkk(); // RND Vx, kk
    void OP_Dxyn(); // DRW Vx, Vy, nibble
    void OP_Ex9E(); // SKP Vx 
    void OP_ExA1(); // SKNP Vx
    void OP_Fx07(); // LD Vx, DT
    void OP_Fx0A(); // LD Vx, K
    void OP_Fx15(); // LD DT, Vx
    void OP_Fx18(); // LD ST, Vx
    void OP_Fx1E(); // ADD I, Vx
    void OP_Fx29(); // LD F, Vx
    void OP_Fx33(); // LD B, Vx
    void OP_Fx55(); // LD [I], Vx
    void OP_Fx65(); // LD Vx, [I]


    // Helpers
    byte D_Opd_00x0();
    byte D_Opd_000x();
    byte D_Opd_0x00();
    byte D_Opd_x000();
    byte D_Opd_xx00();
    byte D_Opd_00xx();
    byte D_Opd_0xx0();
    doubleByte D_Opd_0xxx();
    void initFuncPointerTable();
    void printState();
    void printByte(byte x);

    // Function Pointer Table
    typedef void (Chip8::*Chip8func)();
    // using Chip8func = std::function<void(Chip8*)>;
    Chip8func table[0xF + 1];
    Chip8func table0[0xE + 1];
    Chip8func table8[0xE + 1];
    Chip8func tableE[0xE + 1];
    Chip8func tableF[0x65 + 1];
    void Table0();
    void Table8();
    void TableE();
    void TableF();

    public:
    Chip8();
    void Reset(bool shouldLoadRom = false, std::string filename = "");
    bool UpdateTimers();
    int LoadRom(const char *filename);
    void Cycle();
    ~Chip8();
};
#endif