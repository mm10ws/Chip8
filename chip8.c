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

int main(int argc, char *argv[])
{
    init_emulator(argv[1]);

//    for (;;)
//    {
//        execute_cycle();
//    }
    printf("Hello World!\n");
    return 0;
}

// Sets all registers to their initial values
void init_emulator(char * path_to_rom)
{
    // the program starts at 0x200
    reg_pc = 0x200;

    printf("loading %s\n", path_to_rom);
    FILE *file = fopen(path_to_rom, "rb");
    if (file != NULL)
    {
        fread(memory + 0x200, 1, sizeof(memory) - 0x200, file);
    }
    else
    {
        perror("unable to open file\n");
        exit(EXIT_FAILURE);
    }
}

// execute a single cycle: fetch, decode, and execute
void execute_cycle()
{
    // fetch opcode (left shift the first byte and or it with the second byte)
    // todo: make sure we dont go out of bounds
    // todo: need to increment pc somewhere
    current_opcode =  memory[reg_pc] << 8 | memory[reg_pc + 1];
    decode_and_execute(current_opcode);
}

void decode_and_execute(uint16_t opcode)
{
    // get each nibble of the opcode to decode
    uint8_t nibbles[4];
    nibbles[0] = (opcode >> 12) & 0xf;
    nibbles[1] = (opcode >> 8) & 0xf;
    nibbles[2] = (opcode >> 4) & 0xf;
    nibbles[3] = opcode & 0xf;

    if (opcode == 0x00e0)
    {
        clear_display();
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
    else if (nibbles[0] == 0x4)
    {
        skip_if_reg_not_equals_value(nibbles[1], opcode);
    }
    else if (nibbles[0] == 0x5 && nibbles[3] == 0x0)
    {
        skip_if_reg_equal(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x6)
    {
        load_value(nibbles[1], opcode);
    }
    else if (nibbles[0] == 0x7)
    {
        add_value(nibbles[1], opcode);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x0)
    {
        load_from_register(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x1)
    {
        or_registers(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x2)
    {
        and_registers(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x3)
    {
        xor_registers(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x4)
    {
        add_registers(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x5)
    {
        sub_registers(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x6)
    {
        shift_register_right(nibbles[1]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0x7)
    {
        subn_registers(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0x8 && nibbles[3] == 0xe)
    {
        shift_register_left(nibbles[1]);
    }
    else if (nibbles[0] == 0x9 && nibbles[3] == 0x0)
    {
        skip_if_reg_not_equal(nibbles[1], nibbles[2]);
    }
    else if (nibbles[0] == 0xa)
    {
        load_i_value(opcode);
    }
    else if (nibbles[0] == 0xb)
    {
        jump_reg_plus_value(opcode);
    }
    else if (nibbles[0] == 0xc)
    {
        set_reg_random_byte(nibbles[1], opcode);
    }
    else if (nibbles[0] == 0xd)
    {
        display_sprite(nibbles[1], nibbles[2], nibbles[3]);
    }
    else if (nibbles[0] == 0xe && nibbles[2] == 0x9 && nibbles[3] == 0xe)
    {
        skip_if_key_pressed(nibbles[1]);
    }
    else if (nibbles[0] == 0xe && nibbles[2] == 0xa && nibbles[3] == 0x1)
    {
        skip_if_key_not_pressed(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x0 && nibbles[3] == 0x7)
    {
        delay_timer_to_reg(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x0 && nibbles[3] == 0xa)
    {
        store_key_press(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x1 && nibbles[3] == 0x5)
    {
        set_delay_timer(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x1 && nibbles[3] == 0x8)
    {
        set_sound_timer(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x1 && nibbles[3] == 0xe)
    {
        add_reg_to_i(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x2 && nibbles[3] == 0x9)
    {
        set_i_sprite_location(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x3 && nibbles[3] == 0x3)
    {
        store_bcd(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x5 && nibbles[3] == 0x5)
    {
        copy_reg_to_mem(nibbles[1]);
    }
    else if (nibbles[0] == 0xf && nibbles[2] == 0x6 && nibbles[3] == 0x5)
    {
        load_reg_from_mem(nibbles[1]);
    }
    else
    {
        perror("unknown opcode\n");
        exit(EXIT_FAILURE);
    }
}

void clear_display()
{
    // todo
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

void skip_if_reg_equals_value(uint8_t x, uint16_t opcode)
{
    // increment the pc by two if Vx = value
    uint8_t value = opcode & 0xff;
    if (reg_vx[x] == value)
    {
        reg_pc += 2;
    }
    reg_pc += 2;
}

void skip_if_reg_not_equals_value(uint8_t x, uint16_t opcode)
{
    // increment the pc by two if Vx != value
    uint8_t value = opcode & 0xff;
    if (reg_vx[x] != value)
    {
        reg_pc += 2;
    }
    reg_pc += 2;
}

void skip_if_reg_equal(uint8_t x, uint8_t y)
{
    // increment the pc by two if Vx = Vy
    if (reg_vx[x] == reg_vx[y])
    {
        reg_pc += 2;
    }
    reg_pc += 2;
}

void load_value(uint8_t x, uint16_t opcode)
{
    // load the last byte of the opcode into Vx
    reg_vx[x] = opcode & 0xff;
    reg_pc += 2;
}

void add_value(uint8_t x, uint16_t opcode)
{
    // add the last byte of the opcode to Vx and store in Vx
    reg_vx[x] += opcode & 0xff;
    reg_pc += 2;
}

void load_from_register(uint8_t x, uint8_t y)
{
    // set Vx to the value in Vy
    reg_vx[x] = reg_vx[y];
    reg_pc += 2;
}

void or_registers(uint8_t x, uint8_t y)
{
    // do a bitwise or of Vx and Vy and store in Vx
    reg_vx[x] |= reg_vx[y];
    reg_pc += 2;
}

void and_registers(uint8_t x, uint8_t y)
{
    // do a bitwise and of Vx and Vy and store in Vx
    reg_vx[x] &= reg_vx[y];
    reg_pc += 2;
}

void xor_registers(uint8_t x, uint8_t y)
{
    // do a bitwise xor of Vx and Vy and store in Vx
    reg_vx[x] ^= reg_vx[y];
    reg_pc += 2;
}

void add_registers(uint8_t x, uint8_t y)
{
    // add Vx and Vy and store in Vx, if it overflows set the carry bit Vf
    if (reg_vx[x] > 0xff - reg_vx[y])
    {
        reg_vx[0xf] = 0x1;
    }
    else
    {
        reg_vx[0xf] = 0x0;
    }
    reg_vx[x] += reg_vx[y];
    reg_pc += 2;
}

void sub_registers(uint8_t x, uint8_t y)
{
    // set Vf to not borrow if Vx > Vy, then subtract Vy from Vx and store in Vx
    if (reg_vx[x] > reg_vx[y])
    {
        reg_vx[0xf] = 0x1;
    }
    else
    {
        reg_vx[0xf] = 0x0;
    }
    reg_vx[x] -= reg_vx[y];
    reg_pc += 2;
}

void shift_register_right(uint8_t x)
{
    // shift Vx right by 1, if the LSB is one store it in Vf
    if (reg_vx[x] % 2 != 0)
    {
        reg_vx[0xf] = 0x1;
    }
    else
    {
        reg_vx[0xf] = 0x0;
    }
    reg_vx[x] >>= 1;
    reg_pc += 2;
}

void subn_registers(uint8_t x, uint8_t y)
{
    // set Vf to not borrow if Vy > Vx, then subtract Vx from Vy and store in Vx
    if (reg_vx[y] > reg_vx[x])
    {
        reg_vx[0xf] = 0x1;
    }
    else
    {
        reg_vx[0xf] = 0x0;
    }
    reg_vx[x] = reg_vx[y] - reg_vx[x];
    reg_pc += 2;
}

void shift_register_left(uint8_t x)
{
    // shift Vx left by 1, if the MSB is one store it in Vf
    reg_vx[0xf] = (reg_vx[x] >> 7) & 0x1;
    reg_vx[x] <<= 1;
    reg_pc += 2;
}

void skip_if_reg_not_equal(uint8_t x, uint8_t y)
{
    // increment the pc by two if Vx != Vy
    if (reg_vx[x] != reg_vx[y])
    {
        reg_pc += 2;
    }
    reg_pc += 2;
}

void load_i_value(uint16_t opcode)
{
    // load the last 12 bits of the opcode into I
    reg_i = opcode & 0xfff;
    reg_pc += 2;
}

void jump_reg_plus_value(uint16_t opcode)
{
    // the pc will be set to V0 + last 12 bits of opcode
    uint16_t value = opcode & 0xfff;
    reg_pc = reg_vx[0] + value;
    reg_pc += 2;
}

void set_reg_random_byte(uint8_t x, uint16_t opcode)
{
    // set Vx to be a random byte anded with the last byte of the opcode
    uint8_t value = opcode & 0xff;
    reg_vx[x] = rand() & value;
    reg_pc += 2;
}

void display_sprite(uint8_t x, uint8_t y, uint8_t n)
{
    // todo
}

void skip_if_key_pressed(uint8_t x)
{
    // todo
}

void skip_if_key_not_pressed(uint8_t x)
{
    // todo
}

void delay_timer_to_reg(uint8_t x)
{
    reg_vx[x] = reg_delay;
    reg_pc += 2;
}

void store_key_press(uint8_t x)
{
    // todo
    // reg_vx[x] = reg_delay;
}

void set_delay_timer(uint8_t x)
{
    reg_delay = reg_vx[x];
    reg_pc += 2;
}

void set_sound_timer(uint8_t x)
{
    reg_sound = reg_vx[x];
    reg_pc += 2;
}

void add_reg_to_i(uint8_t x)
{
    reg_i += reg_vx[x];
    reg_pc += 2;
}

void set_i_sprite_location(uint8_t x)
{
    // todo
}

void store_bcd(uint8_t x)
{
    // get decimal value of Vx and store it in memory as BCD at I, I+1, I+2
    memory[reg_i] = reg_vx[x] / 100;
    memory[reg_i + 1] = (reg_vx[x] / 10) % 10;
    memory[reg_i + 2] = reg_vx[x] % 10;
    reg_pc += 2;
}

void copy_reg_to_mem(uint8_t x)
{
    // copies registers V0 to Vx to memory starting at I
    for (int i = 0; i <= x; i++)
    {
        memory[reg_i + i] = reg_vx[i];
    }
    reg_pc += 2;
}

void load_reg_from_mem(uint8_t x)
{
    // load registers V0 to Vx from memory starting at I
    for (int i = 0; i <= x; i++)
    {
        reg_vx[i] = memory[reg_i + i];
    }
    reg_pc += 2;
}
