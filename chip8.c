#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL2/SDL.h>
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

// display information 64x32 display
uint8_t display[64 * 32];

// opcode 2 bytes
uint16_t current_opcode;

// maps key to enum value (SDL library) by index
static int sdl_keymapping[16] =
{
    SDL_SCANCODE_0,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_5,
    SDL_SCANCODE_6,
    SDL_SCANCODE_7,
    SDL_SCANCODE_8,
    SDL_SCANCODE_9,
    SDL_SCANCODE_A,
    SDL_SCANCODE_B,
    SDL_SCANCODE_C,
    SDL_SCANCODE_D,
    SDL_SCANCODE_E,
    SDL_SCANCODE_F
};

static uint8_t fontset[80] =
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

SDL_Window *window;
SDL_Renderer *renderer;

int main(int argc, char *argv[])
{
    init_emulator(argv[1]);

    for (;;)
    {
        execute_cycle();
        usleep(16000);

        if (reg_delay > 0)
        {
            reg_delay--;
        }
        if (reg_sound > 0)
        {
            reg_sound--;
        }
    }
    return 0;
}

// Sets all registers to their initial values
void init_emulator(char * path_to_rom)
{
    // the program starts at 0x200
    reg_pc = 0x200;

    // load fontset into memory
    for (int i = 0; i < 80; i++)
    {
        memory[i] = fontset[i];
    }

    // load rom into memory
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

    // set up SDL for display output
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        perror(SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_CreateWindowAndRenderer(640, 320, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

void draw_display()
{
    for (int i = 0; i < 640; i++)
    {
        for (int j = 0; j < 320; j++)
        {
            if (display[i/10 + 64 * (j / 10)] == 0x1)
            {
                SDL_RenderDrawPoint(renderer, i, j);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

// execute a single cycle: fetch, decode, and execute
void execute_cycle()
{
    // fetch opcode (left shift the first byte and or it with the second byte)
    current_opcode =  memory[reg_pc] << 8 | memory[reg_pc + 1];
    printf("current op: %x\n", current_opcode);
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
    memset(display, 0, sizeof(display));
    draw_display();
}

void return_instruction()
{
    // sets the program counter to the address at the top of the stack,
    // then subtracts 1 from the stack pointer
    reg_sp--;
    reg_pc = stack[reg_sp];
    reg_pc += 2;
}

void jump_instruction(uint16_t opcode)
{
    // set the pc to the last 12 bits of the opcode
    reg_pc = opcode & 0xfff;
}

void call_instruction(uint16_t opcode)
{
    // increment sp and store pc on stack, then call jump
    stack[reg_sp] = reg_pc;
    reg_sp++;
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
    reg_vx[0xf] = 0;
    for (int i = 0; i < n; i++)
    {
        uint8_t pixel = memory[reg_i + i];
        for (int j = 0; j < 8; j++)
        {
            uint8_t pixel_bit = (pixel & (0x80 >> j)) >> (7 - j);
            uint8_t display_bit = display[x + i + 64 * (y + j)];
            if ((pixel_bit & display_bit) == 0x1)
            {
                reg_vx[0xf] = 1;
            }
            display[x + i + 64 * (y + j)] ^= pixel_bit;
        }
    }
    reg_pc += 2;
    draw_display();
}

void skip_if_key_pressed(uint8_t x)
{
    // skip the next instruction if the key in Vx is pressed
    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();

    int key = sdl_keymapping[reg_vx[x]];
    if (keys[key])
    {
        reg_pc += 2;
    }
    reg_pc += 2;
}

void skip_if_key_not_pressed(uint8_t x)
{
    // skip the next instruction if the key in Vx is not pressed
    const uint8_t *keys = SDL_GetKeyboardState(NULL);
    SDL_PumpEvents();

    int key = sdl_keymapping[reg_vx[x]];
    if (!keys[key])
    {
        reg_pc += 2;
    }
    reg_pc += 2;
}

void delay_timer_to_reg(uint8_t x)
{
    reg_vx[x] = reg_delay;
    reg_pc += 2;
}

void store_key_press(uint8_t x)
{
    // wait for a valid key to be pressed and store it in Vx
    const uint8_t *keys = SDL_GetKeyboardState(NULL);

    while (1)
    {
        printf("awaiting valid key press\n");
        SDL_PumpEvents();

        for (int i = 0; i < 16; i++)
        {
            if (keys[sdl_keymapping[i]])
            {
                reg_vx[x] = i;
                reg_pc += 2;
                return;
            }
        }
        // todo: may want to add sleep to slow down this loop
    }
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
    // set I to be the location of the font corresponding to the value in Vx
    // since the fonts are 5 bytes long and in memory at 0x0, we can muliply the value
    // in Vx by 5 to get the location in memory
    reg_i = memory[reg_vx[x] * 5];
    reg_pc += 2;
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
