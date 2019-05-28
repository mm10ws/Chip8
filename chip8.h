#ifndef CHIP8_H
#define CHIP8_H

void init_emulator();
void execute_cycle();
void decode(uint16_t opcode);
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

#endif
