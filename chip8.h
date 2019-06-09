#ifndef CHIP8_H
#define CHIP8_H

void init_emulator(char * path_to_rom, bool debug);
void disassemble(uint16_t op, FILE *f);
void draw_display();
void execute_cycle(bool debug);
void write_debug();
void decode_and_execute(uint16_t opcode);
void clear_display();
void return_instruction();
void jump_instruction(uint16_t opcode);
void call_instruction(uint16_t opcode);
void skip_if_reg_equals_value(uint8_t x, uint16_t opcode);
void skip_if_reg_not_equals_value(uint8_t x, uint16_t opcode);
void skip_if_reg_equal(uint8_t x, uint8_t y);
void load_value(uint8_t x, uint16_t opcode);
void add_value(uint8_t x, uint16_t opcode);
void load_from_register(uint8_t x, uint8_t y);
void or_registers(uint8_t x, uint8_t y);
void and_registers(uint8_t x, uint8_t y);
void xor_registers(uint8_t x, uint8_t y);
void add_registers(uint8_t x, uint8_t y);
void sub_registers(uint8_t x, uint8_t y);
void shift_register_right(uint8_t x);
void subn_registers(uint8_t x, uint8_t y);
void shift_register_left(uint8_t x);
void skip_if_reg_not_equal(uint8_t x, uint8_t y);
void load_i_value(uint16_t opcode);
void jump_reg_plus_value(uint16_t opcode);
void set_reg_random_byte(uint8_t x, uint16_t opcode);
void display_sprite(uint8_t x, uint8_t y, uint8_t n);
void skip_if_key_pressed(uint8_t x);
void skip_if_key_not_pressed(uint8_t x);
void delay_timer_to_reg(uint8_t x);
void store_key_press(uint8_t x);
void set_delay_timer(uint8_t x);
void set_sound_timer(uint8_t x);
void add_reg_to_i(uint8_t x);
void set_i_sprite_location(uint8_t x);
void store_bcd(uint8_t x);
void copy_reg_to_mem(uint8_t x);
void load_reg_from_mem(uint8_t x);

#endif
