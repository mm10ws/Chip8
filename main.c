#include <stdio.h>
#include <stdlib.h>

// registers 16 1 byte registers
unsigned char reg_vx[16];

// I register 2 bytes
unsigned short reg_i;

// delay and sound time registers 1 bytes each
unsigned char reg_delay;
unsigned char reg_sound;

// program counter register 2 bytes
unsigned short reg_pc;

// stack pointer register 1 byte
unsigned char reg_sp;

// stack 16 2 byte values
unsigned short stack[16];

// opcode 2 bytes
unsigned short current_opcode;

int main()
{
    init_emulator();

    printf("Hello World!\n");
    printf("%d", reg_pc);
    return 0;
}

// Sets all registers to their initial values
void init_emulator()
{
    // the program starts at 0x200
    reg_pc = 0x200;
}
