// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

	@filled		// either 0 or -1
	M=0
	@tofill		// deafault white screen
	M=0
(MAIN)
	@KBD
	D=M
	@FILL
	D;JNE		// fill screen if key pressed
	@filled
	D=M
	@WIPE		// wipe only if the screen was filled before
	D;JNE
	@MAIN		// keep checking for keyboard inputs
	0;JMP
(FILL)
	@tofill
	M=-1			// fill with black
(LOOPINIT)
	@j
	M=0
(LOOP)
	@j
	D=M
	@8192			// number of 16 bit words to be filled on the screen
	D=D-A
	@STOP
	D;JEQ			// break the loop
	@SCREEN			// fill screen with b/w
	D=A
	@j
	D=D+M
	@pos
	M=D
	@tofill
	D=M
	@pos
	A=M
	M=D
	@j				// incrementing the counter j
	M=M+1
	@LOOP
	0;JMP
(STOP)
	@tofill
	D=M
	@filled			// updating the filled var 
	M=D
	@MAIN
	0;JMP
(WIPE)
	@tofill
	M=0
	@LOOPINIT
	0;JMP