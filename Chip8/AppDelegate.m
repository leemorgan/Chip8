//
//  AppDelegate.m
//  Chip8
//
//  Created by Lee Morgan on 12/25/14.
//  Copyright (c) 2014 Lee Morgan. All rights reserved.
//

#import "AppDelegate.h"
#import "Chip8.h"
#import "Chip8View.h"

@interface AppDelegate () <NSWindowDelegate>

@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet Chip8View *chip8view;
@property (weak) IBOutlet NSMenuItem *pauseResumeMenuItem;

@property (assign) BOOL paused;
@property (strong) NSTimer *cpuTimer, *displayTimer;

@end


@implementation AppDelegate

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
	
	int windowWidth = (int)frameSize.width / 64 * 64;
	return NSMakeSize(windowWidth, frameSize.height);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	[self openROM:self];
	[self.chip8view becomeFirstResponder];
}

- (IBAction)pauseResume:(id)sender {
	
	if (self.paused) {
		[self resume];
	}
	else {
		[self pause];
	}
}

- (void)pause {
	
	self.paused = YES;
	self.pauseResumeMenuItem.title = @"Resume";
	[self.cpuTimer invalidate];
	[self.displayTimer invalidate];
}

- (void)resume {
	
	self.paused = NO;
	self.pauseResumeMenuItem.title = @"Pause";
	self.cpuTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/800.0 target:self selector:@selector(updateChip8:) userInfo:nil repeats:YES];
	self.displayTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/60.0 target:self selector:@selector(updateDisplay) userInfo:nil repeats:YES];
}

- (void)updateChip8:(NSTimer*)timer {
	
	emulateCycle();
}

- (void)updateDisplay {
	
	if (needsDisplay() == true) {
		[self.chip8view setNeedsDisplay:YES];
	}
}

- (IBAction)openROM:(id)sender {
	
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	
	[self pause];
	
	[openPanel beginWithCompletionHandler:^(NSInteger result) {
		
		if (result == NSFileHandlingPanelOKButton) {
			NSURL *fileURL = [openPanel URL];
			
			NSString *path = [fileURL path];
			loadROM([path cStringUsingEncoding:NSUTF8StringEncoding]);
			
			[self resume];
		}
	}];
}

@end
