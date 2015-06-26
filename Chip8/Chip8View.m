//
//  Chip8View.m
//  Chip8
//
//  Created by Lee Morgan on 12/25/14.
//  Copyright (c) 2014 Lee Morgan. All rights reserved.
//

#import "Chip8View.h"
#import	"Chip8.h"


@implementation Chip8View

- (void)drawRect:(NSRect)dirtyRect {
	
	int pixelWidth =  self.bounds.size.width / 64;
	int pixelHeight = self.bounds.size.height / 32;
	
	[[NSColor blackColor] setFill];
	NSRectFill(self.bounds);
	
	[[NSColor whiteColor] setFill];
	
	for (int row = 0; row < 32; row++) {
		for (int col = 0; col < 64; col++) {
			
			if (gfx[col][row] != 0) {
				NSRect pixelRect = NSMakeRect(col * pixelWidth,
											  self.bounds.size.height - pixelHeight - row * pixelHeight,
											  pixelWidth,
											  pixelHeight);
				
				NSRectFill(pixelRect);
			}
		}
	}
	
	chip8_setNeedsDisplay(false);
}

- (BOOL)canBecomeKeyView {
	return YES;
}

- (BOOL)becomeFirstResponder {
	return YES;
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
	
	NSString *characters;
	characters = [theEvent characters];
	unichar character = [characters characterAtIndex: 0];
	
	switch (character) {
		case '1':
			chip8_keydown('1');
			break;
		case '2':
			chip8_keydown('2');
			break;
		case '3':
			chip8_keydown('3');
			break;
		case '4':
			chip8_keydown('C');
			break;
		case 'q':
			chip8_keydown('4');
			break;
		case 'w':
			chip8_keydown('5');
			break;
		case 'e':
			chip8_keydown('6');
			break;
		case 'r':
			chip8_keydown('D');
			break;
		case 'a':
			chip8_keydown('7');
			break;
		case 's':
			chip8_keydown('8');
			break;
		case 'd':
			chip8_keydown('9');
			break;
		case 'f':
			chip8_keydown('E');
			break;
		case 'z':
			chip8_keydown('A');
			break;
		case 'x':
			chip8_keydown('0');
			break;
		case 'c':
			chip8_keydown('B');
			break;
		case 'v':
			chip8_keydown('F');
			break;
		default:
			NSLog(@"Unrecognized key");
			break;
	}
}

- (void)keyUp:(NSEvent *)theEvent {
	
	NSString *characters;
	characters = [theEvent characters];
	unichar character = [characters characterAtIndex: 0];
	
	switch (character) {
		case '1':
			chip8_keyup('1');
			break;
		case '2':
			chip8_keyup('2');
			break;
		case '3':
			chip8_keyup('3');
			break;
		case '4':
			chip8_keyup('C');
			break;
		case 'q':
			chip8_keyup('4');
			break;
		case 'w':
			chip8_keyup('5');
			break;
		case 'e':
			chip8_keyup('6');
			break;
		case 'r':
			chip8_keyup('D');
			break;
		case 'a':
			chip8_keyup('7');
			break;
		case 's':
			chip8_keyup('8');
			break;
		case 'd':
			chip8_keyup('9');
			break;
		case 'f':
			chip8_keyup('E');
			break;
		case 'z':
			chip8_keyup('A');
			break;
		case 'x':
			chip8_keyup('0');
			break;
		case 'c':
			chip8_keyup('B');
			break;
		case 'v':
			chip8_keyup('F');
			break;
	}
}

@end
