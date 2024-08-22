//
// Created by Aaron Han on 2024-05-20.
//

#include "Chip8.h"
#include <fstream>
#include <chrono>
#include <cstring>
#include <iostream>
#include <SDL.h>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

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

Chip8::Chip8()
        : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    pc = START_ADDRESS;

    // Load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Initialize RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

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

    for (uint8_t tab_index = 0; tab_index < 0xE + 1; tab_index++){
        table0[tab_index] = NULL;
        table8[tab_index] = NULL;
        tableE[tab_index] = NULL;
    }

    // make function calls for table0
    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    // make function calls for table8
    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    // make function calls for tableE
    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    for (uint8_t tab_index = 0; tab_index < 0x65 + 1; tab_index++){
        tableF[tab_index] = NULL;
    }

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::LoadROM(char const* filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()){
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        file.seekg(0,std::ios::beg);
        file.read(buffer,size);
        file.close();

        for (long i=0;i<size;++i){
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }
}

void Chip8::Cycle(){
    opcode = memory[pc] << 8u | memory[pc+1];

    pc += 2;

    (this->*(table[(opcode & 0xF000u) >> 12u]))();
    if (delayTimer>0){
        delayTimer--;
    }

    if (soundTimer>0) {
        soundTimer--;
    }
}


void Chip8::Table0(){
    (this->*(table0[opcode & 0x000Fu]))();
};

void Chip8::Table8(){
    (this->*(table8[opcode & 0x000Fu]))();
};

void Chip8::TableE(){
    (this->*(tableE[opcode & 0x000Fu]))();
};

void Chip8::TableF(){
    (this->*(tableF[opcode & 0x00FFu]))();
};

void Chip8::OP_NULL(){}

void Chip8::OP_00E0(){
    memset(video,0,sizeof(video));
}

void Chip8::OP_00EE(){
    --sp;
    pc = stack[sp];
}

void Chip8::OP_1nnn(){
    uint16_t address = opcode & 0x0FFFu;
    pc = address;
};

void Chip8::OP_2nnn(){
    uint16_t address = opcode & 0x0FFFu;
    stack[sp] = pc;
    pc = address;
    ++sp;
}

void Chip8::OP_3xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = (opcode & 0x00FFu);

    if (registers[Vx]==kk){
        pc += 2;
    }
}

void Chip8::OP_4xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = (opcode & 0x00FFu);

    if (registers[Vx]!=kk){
        pc += 2;
    }
}

void Chip8::OP_5xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx]==registers[Vy]){
        pc += 2;
    }
}

void Chip8::OP_6xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = (opcode & 0x00FFu);

    registers[Vx] = kk;
}

void Chip8::OP_7xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = (opcode & 0x00FFu);

    registers[Vx] += kk;
}


void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vx] | registers[Vy];
    registers[0xF] = 0;
}

void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vx] & registers[Vy];
    registers[0xF] = 0;
}

void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vx] ^ registers[Vy];
    registers[0xF] = 0;
}

void Chip8::OP_8xy4(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint16_t the_sum = registers[Vx] + registers[Vy];
    registers[Vx] = the_sum & 0xFFu;
    if (the_sum>255U){
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

void Chip8::OP_8xy5(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t c = registers[Vx];
    registers[Vx] -= registers[Vy];

    registers[0xF] = 1;
    if (c>registers[Vy]){
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

void Chip8::OP_8xy6(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vy];

    uint8_t c = (registers[Vx] & 0x1u);

    registers[Vx] >>= 1;
    registers[0xF] = c;
}

void Chip8::OP_8xy7(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    bool c = registers[Vx]<=registers[Vy];
    registers[Vx] = registers[Vy] - registers[Vx];

    if (c){
        registers[0xF] = 1;
    } else {
        registers[0xF] = 0;
    }
}

void Chip8::OP_8xyE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vy];

    uint8_t c = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1;
    registers[0xF] = c;
}

void Chip8::OP_9xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    if (registers[Vx]!=registers[Vy]){
        pc += 2;
    }
}

void Chip8::OP_Annn(){
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

void Chip8::OP_Bnnn(){
    uint16_t address = opcode & 0x0FFFu;

    index = address + registers[0x0];
}

void Chip8::OP_Cxkk(){
    uint16_t Vx = (opcode & 0x0F00u) >> 8u;
    uint16_t kk = (opcode & 0x00FFu);

    registers[Vx] = kk & randByte(randGen);
}

void Chip8::OP_Dxyn(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row=0;row<height;row++){
        if ((yPos+row)>=VIDEO_HEIGHT){
            break;
        }
        uint8_t sprByte = memory[index+row];
        for (unsigned int column = 0; column<8; column++){
            if ((xPos+column)>=VIDEO_WIDTH){
                break;
            }
            uint8_t spritePixel = sprByte & (0x80u>>column);
            uint32_t* screenPixel = &video[(yPos+row)*VIDEO_WIDTH + (xPos+column)];
            if (spritePixel){
                if (*screenPixel == 0xFFFFFFFF){
                    registers[0xF] = 1;
                }
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (keypad[key]){
        pc += 2;
    }
}
void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (!keypad[key]){
        pc += 2;
    }
}

void Chip8::OP_Fx07(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A() {
    uint8_t Vx =   (opcode & 0x0F00u) >> 8u;

    SDL_Event event;
    Sint32 key;
    int key_num;

    if (keypad[0]){
        key = SDLK_x;
        key_num = 0;
    }
    else if (keypad[1]){
        key = SDLK_1;
        key_num = 1;
    }
    else if (keypad[2]){
        key = SDLK_2;
        key_num = 2;
    }
    else if (keypad[3]){
        key = SDLK_3;
        key_num = 3;
    }
    else if (keypad[4]){
        key = SDLK_q;
        key_num = 4;
    }
    else if (keypad[5]){
        key = SDLK_w;
        key_num = 5;
    }
    else if (keypad[6])
    {
        key = SDLK_e;
        key_num = 6;
    }
    else if (keypad[7])
    {
        key = SDLK_a;
        key_num = 7;
    }
    else if (keypad[8])
    {
        key = SDLK_s;
        key_num = 8;
    }
    else if (keypad[9])
    {
        key = SDLK_d;
        key_num = 9;
    }
    else if (keypad[10])
    {
        key = SDLK_z;
        key_num = 10;
    }
    else if (keypad[11])
    {
        key = SDLK_c;
        key_num = 11;
    }
    else if (keypad[12])
    {
        key = SDLK_4;
        key_num = 12;
    }
    else if (keypad[13])
    {
        key = SDLK_r;
        key_num = 13;
    }
    else if (keypad[14])
    {
        key = SDLK_f;
        key_num = 14;
    }
    else if (keypad[15])
    {
        key = SDLK_v;
        key_num = 15;
    }
    else
    {
        pc -= 2;
        return;
    }
    while (SDL_PollEvent(&event)){
        if (event.type==SDL_KEYUP && event.key.keysym.sym==key){
            registers[Vx] = key_num;
            break;
        }
    }
}

void Chip8::OP_Fx15(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    delayTimer = registers[Vx];
}

void Chip8::OP_Fx18(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    index += registers[Vx];
}

void Chip8::OP_Fx29(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index = FONTSET_START_ADDRESS + 5*registers[Vx];
}

void Chip8::OP_Fx33(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t hund = registers[Vx] / 100;
    uint8_t ten = (registers[Vx] - 100*hund)/10;
    uint8_t one = (registers[Vx] - 100 * hund - 10 * ten);

    memory[index] = hund;
    memory[index+1] = ten;
    memory[index+2] = one;
}

void Chip8::OP_Fx55(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for (uint8_t registers_index = 0; registers_index <= Vx; registers_index++){
        memory[index + registers_index] = registers[registers_index];
    }
    index += Vx+1;
}

void Chip8::OP_Fx65(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for (uint8_t registers_index = 0; registers_index <= Vx; registers_index++){
        registers[registers_index] = memory[registers_index+index];
    }
    index += Vx+1;
}
