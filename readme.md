# Chip8 Emulator

A simple chip8 emulator written in C

## Dependencies:

The emulator uses the SLD2 library for taking input and drawing to a window. Debian based systems can just do "sudo apt-get install libsdl2-dev" to install. See https://wiki.libsdl.org/Installation for more info.

## Building:

cd to the directory with the source file and run make.

## Usage: 

Specify the path to the chip8 ROM to run and whether the emulator should run in debug mode or not.
Debug mode does the following:
*The emulator will disassemble the ROM and write the output to a text file.
*The emulator will write the state of every register after every instruction to a text file.

```
./chip8 [full path to rom] [debug:true/false]
```

## Example Usage:

Runs the emulator in debug mode:

```
./chip8 /home/username/chip8_rom true
```
