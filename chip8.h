#ifndef CHIP8_H
#define CHIP8_H

void init_emulator();
void execute_cycle();
void decode(uint16_t opcode);
void return_instruction();
void jump_instruction(uint16_t opcode);
void call_instruction(uint16_t opcode);
void skip_if_reg_equals_value(uint8_t reg, uint16_t opcode);

#endif
