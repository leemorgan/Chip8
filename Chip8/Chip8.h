//
//  Chip8.h
//  Chip8
//
//  Created by Lee Morgan on 12/23/14.
//  Copyright (c) 2014 Lee Morgan. All rights reserved.
//

#ifndef __Chip8__Chip8__
#define __Chip8__Chip8__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>


void loadROM(const char *romPath);

void emulateCycle();

void keydown(unsigned char k);
void keyup(unsigned char k);

extern unsigned char gfx[64][32]; // we expose the graphics buffer so the Chip8View can read from it to render to the screen.


#endif /* defined(__Chip8__Chip8__) */
