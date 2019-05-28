#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chip8.h"

// registers 16 1 byte registers
uint8_t reg_vx[16];

// I register 2 bytes
uint16_t reg_i;

// delay and sound time registers 1 bytes each
uint8_t reg_delay;
uint8_t reg_sound;

// program counter register 2 bytes
uint16_t reg_pc;

// stack pointer register 1 byte
uint8_t reg_sp;

// stack 16 2 byte values
uint16_t stack[16];

// memory 4K bytes
uint8_t memory[4096];

// opcode 2 bytes
uint16_t current_opcode;

int main()
{
    init_emulator();

    printf("Hello World!\n");
    return 0;
}

// Sets all registers to their initial values
void init_emulator()
{
    // the program starts at 0x200
    reg_pc = 0x200;
}

// execute a single cycle: fetch, decode, and execute
void execute_cycle()
{
    // fetch opcode (left shift the first byte and or it with the second byte)
    // todo: make sure we dont go out of bounds
    // todo: need to increment pc somewhere
    current_opcode =  memory[reg_pc] << 8 | memory[reg_pc + 1];

    // decode opcode
    decode(current_opcode);
}

void decode(uint16_t opcode)
{
    // get each nibble of the opcode to decode
    uint8_t nibbles[4];
    nibbles[0] = (opcode >> 12) & 0xf;
    nibbles[1] = (opcode >> 8) & 0xf;
    nibbles[2] = (opcode >> 4) & 0xf;
    nibbles[3] = opcode & 0xf;

    if (opcode == 0x0000)
    {

    }
    else if (opcode == 0x00e0)
    {
        // clear display
    }
    else if (opcode == 0x00ee)
    {
        return_instruction();
    }
    else if (nibbles[0] == 0x1)
    {
        jump_instruction(opcode);
    }
    else if (nibbles[0] == 0x2)
    {
        call_instruction(opcode);
    }
    else if (nibbles[0] == 0x3)
    {
        skip_if_reg_equals_value(nibbles[1], opcode);
    }
}

void return_instruction()
{
    // sets the program counter to the address at the top of the stack,
    // then subtracts 1 from the stack pointer
    reg_pc = stack[reg_sp];
    reg_sp--;
}

void jump_instruction(uint16_t opcode)
{
    // set the pc to the last 12 bits of the opcode
    uint16_t address = opcode & 0xfff;
    reg_pc = address;
}

void call_instruction(uint16_t opcode)
{
    // increment sp and store pc on stack, then call jump
    reg_sp++;
    stack[reg_sp] = reg_pc;
    jump_instruction(opcode);
}

void skip_if_reg_equals_value(uint8_t reg, uint16_t opcode)
{
    // increment the pc by two if Vx = value
    uint8_t value = opcode & 0xff;
    if (reg_vx[reg] == value)
    {
        reg_pc = reg_pc + 2;
    }
}
