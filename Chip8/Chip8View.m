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
	
	setNeedsDisplay(false);
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
			keydown('1');
			break;
		case '2':
			keydown('2');
			break;
		case '3':
			keydown('3');
			break;
		case '4':
			keydown('C');
			break;
		case 'q':
			keydown('4');
			break;
		case 'w':
			keydown('5');
			break;
		case 'e':
			keydown('6');
			break;
		case 'r':
			keydown('D');
			break;
		case 'a':
			keydown('7');
			break;
		case 's':
			keydown('8');
			break;
		case 'd':
			keydown('9');
			break;
		case 'f':
			keydown('E');
			break;
		case 'z':
			keydown('A');
			break;
		case 'x':
			keydown('0');
			break;
		case 'c':
			keydown('B');
			break;
		case 'v':
			keydown('F');
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
			keyup('1');
			break;
		case '2':
			keyup('2');
			break;
		case '3':
			keyup('3');
			break;
		case '4':
			keyup('C');
			break;
		case 'q':
			keyup('4');
			break;
		case 'w':
			keyup('5');
			break;
		case 'e':
			keyup('6');
			break;
		case 'r':
			keyup('D');
			break;
		case 'a':
			keyup('7');
			break;
		case 's':
			keyup('8');
			break;
		case 'd':
			keyup('9');
			break;
		case 'f':
			keyup('E');
			break;
		case 'z':
			keyup('A');
			break;
		case 'x':
			keyup('0');
			break;
		case 'c':
			keyup('B');
			break;
		case 'v':
			keyup('F');
			break;
	}
}

@end
