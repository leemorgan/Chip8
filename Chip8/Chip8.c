//
//  Chip8.c
//  Chip8
//
//  Created by Lee Morgan on 12/23/14.
//  Copyright (c) 2014 Lee Morgan. All rights reserved.
//

#include "Chip8.h"

/* 
 Chip8 Architecture:
 35 opcodes.
 All opcodes are two bytes long. The most significant byte is stored first.
 4KB memory
 Fifteen 8-bit general purpose registers named V0,V1...VE
 One 1-bit register used for the 'carry flag' in math operations
 Index Register (I) and Program Counter (PC) can have a value from 0x000 - 0xFFF (0 - 4095). That is to say, they can point at any location in the 4KB of memory.
 
 The system's memory map
 0x000 - 0x1FF : Where the Chip8 interpreter is stored (Lower 80 bytes are used for the built in 4x5 pixel font set (0,1,2...9,A,B...F))
 0x200 - 0xFFF : Program ROM and RAM
 
 Note: Historicaly the CHIP-8 interpreter itself occupies the first 512 bytes (0x000-0x1FF) of the memory space.
 For this reason, programs written for the original system begin at memory location 512 (0x200) and do not access any of the memory below that location.
 
 Graphics:
 Chip8 has one instruction that draws sprite to the screen.
 Drawing is done in XOR mode and if a pixel is turned off as a result of drawing, the VF register is set. This is used for collision detection.
 The graphics of the Chip8 are black and white and the screen has a total of 2048 pixels (64 x 32).
 Graphics are drawn to the screen solely by drawing sprites, which are 8 pixels wide and may be from 1 to 15 pixels in height. 
 Sprite pixels that are set flip the color of the corresponding screen pixel, while unset sprite pixels do nothing. 
 The carry flag (VF) is set to 1 if any screen pixels are flipped from set to unset when a sprite is drawn and set to 0 otherwise.
 
 Chip8 doesn't have any interupt or hardware registers.
 
 There are two timer registers that count at 60Hz. When set above zero they will count down to zero.
 The system's buzzer sounds whenever the sound timer reaches zero.
 
 It is important to note that the Chip8 instruction set has opcodes that allow the program to jump to a certain address, or call a subroutine.
 The stack is used to remember the current location before a jump is performed. So anytime we perform a jump, or call a subroutine, we must store the program counter in the stack before proceeding.
 The system has up to 16 levels of stack and in order to remember which level of the stack is used, we need to implement a stack pointer (sp).
 
 The Chip8 system has a HEX based keypad (0x0 - 0xF).
 
 Opcode Table
 • NNN		12-bit address
 • NN		8-bit constant
 • N		4-bit constant
 • X and Y	4-bit register identifier
 --------------------------------------------------
 0NNN	Calls RCA 1802 program at address NNN. (not implemented)
 00E0	Clears the screen.
 00EE	Returns from a subroutine.
 1NNN	Jumps to address NNN.
 2NNN	Calls subroutine at NNN.
 3XNN	Skips the next instruction if VX equals NN.
 4XNN	Skips the next instruction if VX doesn't equal NN.
 5XY0	Skips the next instruction if VX equals VY.
 6XNN	Sets VX to NN.
 7XNN	Adds NN to VX.
 8XY0	Sets VX to the value of VY.
 8XY1	Sets VX to VX or VY.
 8XY2	Sets VX to VX and VY.
 8XY3	Sets VX to VX xor VY.
 8XY4	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
 8XY5	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
 8XY6	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
 8XY7	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
 8XYE	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
 9XY0	Skips the next instruction if VX doesn't equal VY.
 ANNN	Sets I to the address NNN.
 BNNN	Jumps to the address NNN plus V0.
 CXNN	Sets VX to a random number and NN.
 DXYN	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn't change after the execution of this instruction. VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen
 EX9E	Skips the next instruction if the key stored in VX is pressed.
 EXA1	Skips the next instruction if the key stored in VX isn't pressed.
 FX07	Sets VX to the value of the delay timer.
 FX0A	A key press is awaited, and then stored in VX.
 FX15	Sets the delay timer to VX.
 FX18	Sets the sound timer to VX.
 FX1E	Adds VX to I.
 FX29	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
 FX33	Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
 FX55	Stores V0 - VX in memory starting at address I.
 FX65	Fills V0 - VX with values from memory starting at address I.
*/



// Convenience macros to make parsing the opcode easier (and more readable)
#define GetX(opcode)	((opcode & 0x0F00) >> 8)	// X is a register identifer. It is stored in the second nibble in the opcode.
#define GetY(opcode)	((opcode & 0x00F0) >> 4)	// Y is a register identifer. It is stored in the third nibble in the opcode.
#define GetN(opcode)	( opcode & 0x000F)			// N is a 4-bit (nibble) constant stored in the last nibble of the opcode.
#define GetNN(opcode)	( opcode & 0x00FF)			// NN is a 8-bit (byte) constant stored in the lower byte of the opcode.
#define GetNNN(opcode)	( opcode & 0x0FFF)			// NNN is an address stored in the lower 12 bits of the opcode.



// Used to store the current opcode (2 bytes)
unsigned short opcode;

// Sytem Memory (4 KB)
unsigned char memory[4096];

// Registers
unsigned char	V[16];	// 8-bit registers
#define VF		V[0xF]	// VF (register 16) is a special register reserved for carry operations. So we define a convience accessor here.

// Delay Timer register
unsigned char delay_timer;

// Sound Timer register
unsigned char sound_timer;

// Index Register (sometimes called Address register) is 2 bytes wide and is used with several opcodes that involve memory operations.
unsigned short I;

// Program Coutner (PC) - 2 bytes wide, so we can point at any valid memory location (0-4095) in the 4KB of memory. Technically we only need 12-bits for this.
unsigned short pc;

// VRAM (the screen memory)
unsigned char gfx[64][32];

// The Stack, and Stack Pointer (sp)
unsigned short stack[16];
unsigned short sp;

// Keypad - used as input from a HEX keypad
unsigned char key[16];

// Font
// The Chip8 font is an array of bytes, each one representing one row of pixels
// I've put the first few characters in binary form to show this.
// If you look closely at the 1's you'll see the character they represent.
unsigned char chip8_fontset[80] = {
	
	0b11110000, // 0
	0b10010000,
	0b10010000,
	0b10010000,
	0b11110000,
	
	0b00100000, // 1
	0b01100000,
	0b00100000,
	0b00100000,
	0b01110000,
	
	0b11110000, // 2
	0b00010000,
	0b11110000,
	0b10000000,
	0b11110000,
	
	0b11110000, // 3
	0b00010000,
	0b11110000,
	0b00010000,
	0b11110000,
	
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
	0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};


// Flag used to let the renderer (Chip8View) know when we have updated the graphics memory.
bool _needsDisplay = false;


// Function Prototypes
void chip8init();
void unknownOpcode();

// We use NSBeep() to play a tone when the soundTimer ends.
// Instead of importing all of AppKit we just declare it's prototype here to keep the compiler from complaining.
void NSBeep(void);



void loadROM(const char *romPath) {
	
	// Init the Chip8 system
	chip8init();
	
	
	// Load the game into memory
	// The first 512 bytes of memory are reserved, so we can't load more than 3584 bytes (3.5KB of our total 4KB memory)
	FILE *rom = fopen(romPath, "r");
	if (rom != NULL) {
		
		size_t result = fread(&memory[512], sizeof(unsigned char), 3584, rom);
		
		if (result == -1) {
			printf("Failed to read rom into memory with error %zu\n", result);
		}
	}
	fclose(rom);
}


void chip8init() {
	
	// init the registers and memory
	pc		= 0x200;	// program counter starts at 0x200
	opcode	= 0;		// zeroize the opcode
	I		= 0;		// zeroize the index register
	sp		= 0;		// zeroize the stack pointer
	
	// clear the display
	for (int row = 0; row < 32; row++) {
		for (int col = 0; col < 64; col++) {
			gfx[col][row] = 0;
		}
	}
	
	// clear the stack
	for (int i = 0; i < 16; ++i) {
		stack[i] = 0;
	}
	
	// clear the keys
	for (int i = 0; i < 16; i++) {
		key[i] = V[i] = 0;
	}
	
	// clear registers V0-VF
	for (int i = 0; i < 16; i++) {
		V[i] = 0;
	}
	
	// clear memory
	for (int i = 0; i < 4096; i++) {
		memory[i] = 0;
	}
	
	// Load fontset
	for (int i = 0; i < 80; i++) {
		memory[i] = chip8_fontset[i];
	}
	
	// reset timers
	delay_timer = 0;
	sound_timer = 0;
	
	// seed random
	srandom((unsigned int)time(NULL));
}


void emulateCycle() {
	
	// fetch opcode
	// fetch one opcode from the memory at the location specified by the program counter (pc).
	// In our emulator, data is stored in an array in which each address contains one byte.
	// As one opcode is 2 bytes long, we need to fetch two successive bytes and merge them to get the actual opcode.
	opcode = memory[pc];		// get the first byte
	opcode <<= 8;				// shift the first byte to the left 1 byte (to make room for the 2nd byte)
	opcode |= memory[pc + 1];	// get the 2nd byte
	
	// decode & execute opcode
	// decode the opcode and check the opcode table to see what it means.
	// because every instruction is 2 bytes long, we need to increment the program counter by two after every executed opcode.
	// This is true unless you jump to a certain address in the memory or if you call a subroutine (in which case you need to store the program counter in the stack).
	// If the next opcode should be skipped, increase the program counter by four.
	
	// Get the first nibble of the opcode using a mask like `0xF000. This allows us to "categorize" the opcode
	// Then based on the category, apply more masks to either get the register values from the opcode, or whatever other variables might be encoded with the opcode that will narrow down the actual function that would need to be called, as well as it's arguments.
	
	unsigned short maskedOpcode = opcode & 0xF000;
//	printf("opcode			= 0x%X\n", opcode);			// uncomment to print the current opcode
//	printf("masked opcode	= 0x%X\n", maskedOpcode);	// uncomment to print the current masked opcode
	
	if (opcode == 0x00E0) {
		for (int row = 0; row < 32; row++) {
			for (int col = 0; col < 64; col++) {
				gfx[col][row] = 0;
			}
		}
		pc += 2;
	}
	else if (opcode == 0x00EE) {
		if (sp <= 0) {
			printf("WARNING: Stack Underflow\n");
			return;
		}
		sp--;
		pc = stack[sp];
	}
	else if (maskedOpcode == 0x1000) {
		pc = GetNNN(opcode);
	}
	else if (maskedOpcode == 0x2000) {
		if (sp+1 > 15) {
			printf("WARNING: Stack Overflow\n");
			return;
		}
		stack[sp] = pc + 2;
		sp++;
		pc = GetNNN(opcode);
	}
	else if (maskedOpcode == 0x3000) {
		unsigned char X = GetX(opcode); // mask to X, then shift 8 (256 bits) to drop the 2nd byte (e.g. 0x3100 = [0011 0001][0000 0000] becomes [0000 0001][0000 0000] then [0000 0000][0000 0001])
		unsigned char NN = GetNN(opcode);
		if (V[X] == NN) {
			pc += 4;
		}
		else {
			pc += 2;
		}
	}
	else if (maskedOpcode == 0x4000) {
		unsigned char X = GetX(opcode);
		unsigned char NN = GetNN(opcode);
		if (V[X] != NN) {
			pc += 4;
		}
		else {
			pc += 2;
		}
	}
	else if (maskedOpcode == 0x5000) {
		unsigned char X = GetX(opcode);
		unsigned char Y = GetY(opcode);
		if (V[X] == V[Y]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
	}
	else if (maskedOpcode == 0x6000) {
		unsigned char X = GetX(opcode);
		unsigned char NN = GetNN(opcode);
		V[X] = NN;
		pc += 2;
	}
	else if (maskedOpcode == 0x7000) {
		unsigned char X = GetX(opcode);
		unsigned char NN = GetNN(opcode);
		V[X] += NN;
		pc += 2;
	}
	else if (maskedOpcode == 0x8000) {
		
		unsigned short submaskedOpcode = opcode & 0x000F;
		
		if (submaskedOpcode == 0x0000) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			V[X] = V[Y];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0001) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			V[X] = V[X] | V[Y];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0002) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			V[X] = V[X] & V[Y];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0003) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			V[X] = V[X] ^ V[Y];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0004) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			if (V[Y] > (255 - V[X])) { // since a byte can only store up to 255, we can check for a carry by seeing if the number we are adding is greather than 255 minus somenumber.
				VF = 1;
			}
			else {
				VF = 0;
			}
			V[X] += V[Y];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0005) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			if (V[Y] > V[X]) { // we will only need to borrow if the number we are subtracting from is smaller than the number we are subtracting with
				VF = 0; // borrow
			}
			else {
				VF = 1;
			}
			V[X] -= V[Y];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0006) {
			unsigned char X = GetX(opcode);
			VF = V[X] & 0x1;
			V[X] = V[X] >> 1;
			pc += 2;
		}
		else if (submaskedOpcode == 0x0007) {
			unsigned char X = GetX(opcode);
			unsigned char Y = GetY(opcode);
			if (V[X] > V[Y]) {
				VF = 0; // borrow
			}
			else {
				VF = 1;
			}
			V[X] = V[Y] - V[X];
			pc += 2;
		}
		else if (submaskedOpcode == 0x000E) {
			unsigned char X = GetX(opcode);
			VF = V[X] >> 7;
			V[X] = V[X] << 1;
			pc += 2;
		}
		else {
			unknownOpcode();
		}
	}
	else if (maskedOpcode == 0x9000) {
		unsigned char X = GetX(opcode);
		unsigned char Y = GetY(opcode);
		if (V[X] != V[Y]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
	}
	else if (maskedOpcode == 0xA000) {
		unsigned short NNN = GetNNN(opcode);
		I = NNN;
		pc += 2;
	}
	else if (maskedOpcode == 0xB000) {
		unsigned short NNN = GetNNN(opcode);
		pc = NNN + V[0];
	}
	else if (maskedOpcode == 0xC000) {
		unsigned char X = GetX(opcode);
		unsigned char NN = GetNN(opcode);
		unsigned char randomByte = random();
		V[X] = randomByte & NN;
		pc += 2;
	}
	else if (maskedOpcode == 0xD000) {
		unsigned char X = GetX(opcode);
		unsigned char Y = GetY(opcode);
		unsigned char height = GetN(opcode);
		unsigned char pixel;
		
		VF = 0;
		
		for (unsigned char yLine = 0; yLine < height; yLine++) {
			unsigned char row = yLine + V[Y];
			
			for (unsigned char xLine = 0; xLine < 8; xLine++) {
				// each 'pixel' is one bit. each sprite is 8 pixels wide (1 byte). So we step through each bit checking if it is set.
				pixel = memory[I + yLine] & (0x80 >> xLine);
				
				if (pixel != 0) {
					unsigned char col = xLine + V[X];
					if (gfx[col][row] == 1) {
						VF = 1;
					}
					gfx[col][row] = gfx[col][row] ^ 1;
				}
			}
		}
		
		_needsDisplay = true;
		
		pc += 2;
	}
	else if (maskedOpcode == 0xE000) {
		
		unsigned short submaskedOpcode = opcode & 0x00FF;
		
		if (submaskedOpcode == 0x009E) {
			unsigned char X = GetX(opcode);
			if (key[V[X]] != 0) {
				pc += 4;
			}
			else {
				pc += 2;
			}
		}
		else if (submaskedOpcode == 0x00A1) {
			unsigned char X = GetX(opcode);
			if (key[V[X]] == 0) {
				pc += 4;
			}
			else {
				pc += 2;
			}
		}
		else {
			unknownOpcode();
		}
	}
	else if (maskedOpcode == 0xF000) {
		
		unsigned short submaskedOpcode = opcode & 0x00FF;
		
		if (submaskedOpcode == 0x0007) {
			unsigned char X = GetX(opcode);
			V[X] = delay_timer;
			pc += 2;
		}
		else if (submaskedOpcode == 0x000A) {
			unsigned char X = GetX(opcode);
			bool keyPress = false;
			
			for (unsigned char i = 0; i < 16; i++) {
				if (key[i] != 0) {
					V[X] = i;
					keyPress = true;
				}
			}
			if (!keyPress) {
				// we didn't receive a key press, skip this cycle and try again (that is, don't advance the pc, just loop back to this opcode again)
				return;
			}
			pc += 2;
		}
		else if (submaskedOpcode == 0x0015) {
			unsigned char X = GetX(opcode);
			delay_timer = V[X];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0018) {
			unsigned char X = GetX(opcode);
			sound_timer = V[X];
			pc += 2;
		}
		else if (submaskedOpcode == 0x001E) {
			unsigned char X = GetX(opcode);
			I += V[X];
			pc += 2;
		}
		else if (submaskedOpcode == 0x0029) {
			unsigned char X = GetX(opcode);
			I = V[X] * 5; // font's are 5 bytes, so we can multiple by 5 to move to the start of the font
			pc += 2;
		}
		else if (submaskedOpcode == 0x0033) {
			unsigned char X = GetX(opcode);
			memory[I]	= V[X] / 100;
			memory[I+1]	= (V[X] / 10) % 10;
			memory[I+2]	= V[X] % 10;
			pc += 2;
		}
		else if (submaskedOpcode == 0x0055) {
			unsigned char X = GetX(opcode);
			for (unsigned char r = 0; r <= X; r++) {
				memory[I+r] = V[r];
			}
			pc += 2;
		}
		else if (submaskedOpcode == 0x0065) {
			unsigned char X = GetX(opcode);
			for (unsigned char r = 0; r <= X; r++) {
				V[r] = memory[I+r];
			}
			pc += 2;
		}
		else {
			unknownOpcode();
		}
	}
	else {
		unknownOpcode();
	}
	
	
	// Update Timers
	static struct timeval lastFireTime = {.tv_sec = 0, .tv_usec = 0};
	struct timeval currentTime;
	struct timeval timeDiff;
	
	gettimeofday(&currentTime, NULL);
	timersub(&currentTime, &lastFireTime, &timeDiff);
	double totalTime = (timeDiff.tv_sec * 1000000.0 + timeDiff.tv_usec) / 1000000.0;
	
	if (totalTime >= 1.0/60.0f) {
		
		lastFireTime = currentTime;
		
		if (delay_timer > 0) {
			delay_timer--;
		}
		
		if (sound_timer > 0) {
			NSBeep();
			sound_timer--;
		}
	}
}

void unknownOpcode() {
	
	printf("Unknown opcode: 0x%X at PC: %d\n", opcode, pc);
}

void keydown(unsigned char k) {
	
	switch (k) {
		case '1':
			key[0x1] = 1;
			break;
		case '2':
			key[0x2] = 1;
			break;
		case '3':
			key[0x3] = 1;
			break;
		case '4':
			key[0x4] = 1;
			break;
		case '5':
			key[0x5] = 1;
			break;
		case '6':
			key[0x6] = 1;
			break;
		case '7':
			key[0x7] = 1;
			break;
		case '8':
			key[0x8] = 1;
			break;
		case '9':
			key[0x9] = 1;
			break;
		case '0':
			key[0x0] = 1;
			break;
		case 'A':
			key[0xA] = 1;
			break;
		case 'B':
			key[0xB] = 1;
			break;
		case 'C':
			key[0xC] = 1;
			break;
		case 'D':
			key[0xD] = 1;
			break;
		case 'E':
			key[0xE] = 1;
			break;
		case 'F':
			key[0xF] = 1;
			break;
		default:
			printf("Chip8: Unrecognized key");
			break;
	}
}

void keyup(unsigned char k) {
	
	switch (k) {
		case '1':
			key[0x1] = 0;
			break;
		case '2':
			key[0x2] = 0;
			break;
		case '3':
			key[0x3] = 0;
			break;
		case '4':
			key[0x4] = 0;
			break;
		case '5':
			key[0x5] = 0;
			break;
		case '6':
			key[0x6] = 0;
			break;
		case '7':
			key[0x7] = 0;
			break;
		case '8':
			key[0x8] = 0;
			break;
		case '9':
			key[0x9] = 0;
			break;
		case '0':
			key[0x0] = 0;
			break;
		case 'A':
			key[0xA] = 0;
			break;
		case 'B':
			key[0xB] = 0;
			break;
		case 'C':
			key[0xC] = 0;
			break;
		case 'D':
			key[0xD] = 0;
			break;
		case 'E':
			key[0xE] = 0;
			break;
		case 'F':
			key[0xF] = 0;
			break;
		default:
			printf("Chip8: Unrecognized key");
			break;
	}
}

bool needsDisplay() {
	return _needsDisplay;
}

void setNeedsDisplay(bool needsDisplay) {
	_needsDisplay = needsDisplay;
}
