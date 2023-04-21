/************************************************************************************
  ___      ___       __  ___________  ___________  _______    __      ________
 |"  \    /"  |     /""\("     _   ")("     _   ")/"      \  |" \    /"       )
  \   \  //   |    /    \)__/  \\__/  )__/  \\__/|:        | ||  |  (:   \___/
 |: \.        |  //  __'  \ |.  |        |.  |    //      /  |.  |    __/  \\
 |.  \    /:  | /   /  \\  \\:  |        \:  |   |:  __   \  /\  |\  /" \   :)
 |___|\__/|___|(___/    \___)\__|         \__|   |__|  \___)(__\_|_)(_______/
               ..............................................
               ....########......#########...####......####..
               ...####..####...#####.........######..######..
               ..####....####....########....##############..
               ..############.........#####..####.####.####..
               ..###......###...#########....####..##..####..
               ..............................................
               
               
	Welcome to mattris-asm! This rendition of my mattris project aims to
	reconcile some inefficiencies of the original, cut away some bloat
	caused by ncurses (though some flair is retained in "title.c"), and
	hopefully rework lots of the C code into assembly to take advantage
	of the TetrInteger, a special data type I've conceptualized
	specifically for this project.
	
	The TetrInteger is a bit-packed list of 2-bit coordinates, in 4
	pairs of (y,x). Note that y precedes, as Ncurses deals in
	row-major values. Each Tetris block may be represented by this list,
	in addition to an anchor point in relation to the game's board.
	
	For example, consider the T block. Its conceptual layout and associated
	TetrInteger value may be represented as shown below. Note that the
	four segments of the blocks need not occur in any particular order,
	however packing them as they appear from upper-left to lower-right 
	is preferred. This is because every method of deciphering a block 
	starts from the end of the value, and the segments towards the 
	lower-right of a Tetris piece are more likely to collide with things. 
	
	(y\x) | 00 | 01 | 10 | 11
	   -- ....................
	   00 .    .    .    .   .
	   -- ....................
	   01 . ## . ## . ## .   .
	   -- ....................  --> Becomes yxyxyxyx = 0100010101101000
	   10 .    . ## .    .   .
	   -- ....................
	   11 .    .    .    .   .
	   -- ....................
	   
	Written by Matthew Kenneth Peterson; ID 3719754; github "MKP157"
	
	@ Dr. Jong-Kyou Kim, CS 2253 - Final Project
	@ Last updated April 20, 2023
	
*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include "./defines.h"
#include "./graphics.c"

/**************************************************
*		Global definitions
*
* "ANCHOR"    : Upper-left tile of a Tetris piece.
*		Serves as anchor-point in relation
*		to the game board.
* "level"     : Game difficulty.
* "lines"     : Effective score.
* "board"     : Array of short ints. Each short
*		is a row of the board, where a
*		0 represents an empty space
*		and 1 is a filled space.
* "rowPointer": This pointer is how we pass the board
*		to assembly; it's volatile so that
*		the compiler doesn't remove it for
*		optimization's sake.
* "block"     :	Tetris block in play.
* "possibleBlocks" :
*		Possible block values. Defined in
*		"defines.h".
***************************************************/

unsigned short int 	ANCHOR;
unsigned int		level = 9;
unsigned int		lines = 0;
unsigned int		linesRemaining = 10;

#define BOARDTYPE unsigned short int
BOARDTYPE 		board[20] = {0};
volatile BOARDTYPE 	*rowPointer = &board[0];

#define TETRINTEGER unsigned short int
TETRINTEGER		block = 0;
const TETRINTEGER	possibleBlocks[7] = {block_T, block_I, block_O, block_J, block_L, block_S, block_Z};

// Function fix-up
void sighandler(int);
extern TETRINTEGER ASMrotateBlock ( TETRINTEGER given );

/**************************************************
*		"newBlock"
*
* Places the anchor point in the initial position,
* and selects one of the 7 predefined block 
* possibilities to put into place.
*
* Input  : None
* Output : Assigns generated value to global "block"
***************************************************/
void newBlock()
{
	ANCHOR_RESET;
	block = possibleBlocks[ rand() % 7 ];
}

/**************************************************
*		"drawBlock"
* 
* Draw the current block to the screen. Cycles
* through the coordinates using inline assembly.
*
* Input: 
*	int "x", which selects the drawing mode.
*	Non-zero value indicate to draw the block.
*	Zero indicates to erase.
*	
* Output : None
***************************************************/

void drawBlock(int x)
{
	TETRINTEGER temp = block;
	unsigned int resultX, resultY;
	short int i = 0;
	char* out;
	if (x)
		out = "[]";
	else
		out = " .";
	
	
	mvprintw( 9, 30, "< * BLOCK DRAW DEBUG * >");
	
	_loop:
		/* Original C:
		// 0x3 is a 2-bit mask.
		resultX = temp & 0x3;
		temp >>= 2;
		resultY = temp & 0x3;
		temp >>= 2;*/
		
		/* Assembly rewrite.
		
		Note the "volatile" keyword. This is specifically
		so that the compiler does not remove or attempt
		to optimize the inline-assembly. For all intents
		and purposes, my use of it is fine, however
		I recommend reading the documentation for it
		before using it yourself. 
		
		Trims the last 4 bits off of the temporary block,
		and returns them as the next y and x to be
		evaluated. Then, what's left of the temporary
		block is returned for processing.*/
		
		asm volatile 
		( 
			".intel_syntax noprefix;"
			
			"mov eax, ecx ;"
			"and eax, 0x3 ;"
			"shr ecx, 0x2 ;"
			
			"mov ebx, ecx ;"
			"and ebx, 0x3 ;"
			"shr ecx, 0x2 ;"
			
			".att_syntax ;"
			
			: "=a" ( resultX ), "=b" ( resultY ), "=c" ( temp )
			: "a" (0), "b" (0), "c" ( temp )
			: "cc"
		);
			
		// Debug;
		mvprintw( i+10, 30, "%d,%d : %08x", resultX, resultY, temp );
		
		// Print resulting block fragment:
		mvprintw(ULPY_GET + resultY, (ULPX_GET + resultX) * 2, "%s", out);
		
	i++;
	if (i < 4) goto _loop;

	refresh();
}

/**************************************************
*		"drawBoard"
*
* Called after line-clears. Redraws the board with
* proper respect to colouring, as the static
* board is drawn in a rainbow pattern.
*
* Input  : None
* Output : Current state of board drawn to screen.
***************************************************/
void drawBoard()
{
	int temp1, temp2;
	short int i = 0, j;
	rowLoop:
		move(i, 0);
		
		// Save focused row value into temp.
		temp1 = board[i];
		j = 9;
		
		columnLoop:
			// If the column-value bit of
			// a row is 1, display it.
			// Otherwise, overwrite with
			// an empty space.
			
			temp2 = (board[i] >> j);

			if (temp2 & 0x1)
			{
				// Enable corresponding row's
				// colour for printing.
				attron(COLOR_PAIR( i % 7 + 1 ));
				printw("[]");
				
				// Switch back to white.
				attron(COLOR_PAIR(7));
			}
			else
				printw(" .");
		
			j--;
		if (j >= 0) goto columnLoop;
	
		i++;
	if (i < 20) goto rowLoop;

	refresh();
}

/**************************************************
*		"checkCollide"
*
* Checks for collision in a given direction against
* the game board, for the block specified. This need
* not be the block in play, nor does a valid 
* direction need to be specified. For example, block
* rotation checks to make sure its result fits into 
* the current board-space without overwriting the
* block in play until it has confirmed that it will
* fit.
*
* Input  : 
*	int "dir", which specifies the collision
*	direction to be checked.
*
*	TETRINTEGER "given", a pointer towards
*	whatever block the function-caller
*	would like to evaluate.
*
* Output :
*	integer result. Returning 0 indicates
*	that no collision has been detected.
*	Non-zero means it has.
*
***************************************************/

int checkCollide(int dir, TETRINTEGER *given)
{
	drawBlock(0);
	
	TETRINTEGER temp = *given;
	int bit = 0x0, result = 0;
	int ofs_X = 0, ofs_Y = 0;
	int row, col;
	short int i = 0;
	
	switch (dir)
	{
		case GOLEFT:	ofs_X = -1;	break;

		case GORIGHT:	ofs_X = 1;	break;

		case GODOWN:	ofs_Y = 1;	break;
	
		// we don't need to change the offset with rotation.
		default:
	}
	
	mvprintw(15, 30, "< * COLLISION DEBUG * >");
	
	// Rotation algorithm in raw assembly!
	_Loop:
		asm volatile 
		(
			".intel_syntax noprefix;"
			
			"push rcx ;"		// Preserve the current temporary block
			"and ecx, 0x3 ;"	// Mask the last two bits of temp
			"add eax, ecx ;"	// Add this mask to [anchor X position + directional offset] 
						// (*see ASM parameters below)
						
			"pop rcx ;"		// Retrieve temporary block
			"shr ecx, 0x2 ;"	// Shift temp. 2 bits right
			
			"push rcx ;"		// Repeat above for Y (next 2 bits)
			"and ecx, 0x3 ;"
			"add ebx, ecx ;"
			"pop rcx ;"
			"shr rcx, 0x2 ;"
			
			"push rcx ;"		// Preserve the current temporary block
			
			/**************************************************
			*		Board check
			***************************************************/
			"mov r15, [rdi+2*rbx] ;"// Retrieve the focused row value of the board's array into R15
			
			"mov rcx, 0x9 ;"	
			"sub rcx, rax ;"	
			"shr r15, cl ;"		// Shift right by the inverse of # of columns (rax)
			"and r15, 0x1 ;"	// Mask the important bit!
			
			"mov r14, 0x1 ;"	// Set R14 to 0x1 for comparison
			"cmp r15, r14 ;"	// Compare with R15; if final bit is set to '1', there's a collision.
			"je .collide%= ;"	
			
			/**************************************************
			*		Out-of-bounds checking
			***************************************************/
			"mov r15, 1 ;"		// r15 for conditional move
			"xor ecx, ecx ;"	// Clear C-reg for reuse
			
			"xor edx, edx ;"	// Set edx to 0
			"cmp eax, edx ;"	// If A-reg (X position) less than 0, collide
			"jl .collide%= ;"
			
			"mov rdx, 0x9 ;"	// Set edx to 9
			"cmp eax, edx ;"	// If A-reg (X position) greater than 9, collide
			"jg .collide%= ;"
			
			"xor edx, edx ;"	
			"cmp ebx, edx ;"	// If B-reg (Y position) less than 0, collide
			"jl .collide%= ;"
			
			"mov rdx, 0x13 ;"
			"cmp ebx, edx ;"	// If B-reg (Y position) greater than decimal 19, collide
			"jg .collide%= ;"
			
			"jmp .done%= ;"
			
			".collide%=: ;"
			"mov rcx, r15 ;"
			
			".done%=: ;"
			"mov rax, rcx ;"	// Put result into rax
			"pop rcx ;"		// Restore temp block
			".att_syntax ;"
			
			: "=a" ( result ), "=c" ( temp )
			: "a" ( ULPX_GET + ofs_X ), "b" ( ULPY_GET + ofs_Y ), "c" ( temp ), "d" (0), "D" (board)
			: "cc"
		);
		
		mvprintw( i+16, 30, "result:%d, temp:%04x", result, temp);
		refresh();
		
		if (result) return 1;
		
		i++;
	if (i < 4 && !result) goto _Loop;

	return 0;
}

/**************************************************
*		"writeBlock"
*
* Handles both writing blocks to the game board,
* as well as checking the board afterward for lines 
* to clear.
*
* Input  : None
* Output : None
***************************************************/

void writeBlock(TETRINTEGER *given)
{
	TETRINTEGER temp_block = *given;
	unsigned int temp_col, temp_row;
	int i = 0, k;
	
	// Keep track of affected rows.
	int temp_rows[4] = {0,0,0,0};

	_writeLoop:
		
		asm volatile 
		( 
			".intel_syntax noprefix;"
			
			"push rcx ;"		// Preserve the current temporary block
			"and ecx, 0x3 ;"	// Mask the last two bits of temp
			"add eax, ecx ;"	// Add this mask to [anchor X position] 
			"pop rcx ;"		// Retrieve temporary block
			"shr rcx, 0x2 ;"	// Shift temp. 2 bits right
			
			"push rcx ;"		// Repeat above for Y (next 2 bits)
			"and ecx, 0x3 ;"	
			"add ebx, ecx ;"	
			"pop rcx ;"
			"shr rcx, 0x2 ;"
			
			".att_syntax ;"
			
			: "=a" ( temp_col ), "=b" ( temp_row ), "=c" ( temp_block )
			: "a" ( ULPX_GET ), "b" ( ULPY_GET ), "c" ( temp_block )
			: "cc"
		);
		
		// Gameover/Error condition.
		// It is a part of the block-writing function so that
		// we avoid segmentation faults. Why? Not sure.
		// But it works, and I'm tired.
		
		if ( board[temp_row] & (0x1 << (9-temp_col)) )
		{
			endwin();
			gameOver();
			curs_set(1);
			exit(1);
		}
		
		// Flip affected bit of row value to 1
		board[temp_row] = board[temp_row] | (0x1 << (9-temp_col));
		
		i++;
	
	if (i < 4) goto _writeLoop;
	
	/* Line clearing:
	1. Check row i. 
		If value = binary 1111111111, the row is full. 
		If not, iterate and check next.
	
	2. For each row above current, shift down by 1.
	3. Clear top board row.
	4. Increment line count.
	5. If level is less than nine, set it to the 
	current line count devided by 10.
	6. Repeat for all rows i. */
	
	i = 0;
	_checkLineLoop:
	
		if ( board[i] == 0x3FF )
		{
			k = i;
			
			_clearLineLoop:
				board[k] = board[k-1];
				k--;
			
			if (k > 1) goto _clearLineLoop;
		
			board[0] = 0;
			lines++;
			linesRemaining--;
			
			if ( linesRemaining == 0 && ( level > 0 ) )
			{
				linesRemaining = 10;
				level--;
			}
		}
	
		i++;
		if (i < 20) goto _checkLineLoop;
	
	// Draw resulting board and refresh the screen.
	drawBoard();
	refresh();
}


/**************************************************
*		Alarm signal handler
*
* Every (1/10 second * level) milliseconds, an alarm 
* will interrupt code execution to process the current
* block's gravity.
*
* If a collision beneath the block is NOT detected, 
* the block will move downard and a new alarm will 
* be triggered. Else, a new block will be generated,
* the current one will be written to the board,
* and the process will restart in order to check the
* new block's gravity.
*
* Input  : "SIGALRM", signal code for alarm interrupts.
* Output : None
***************************************************/

void sighandler(int signum)
{
	if (!checkCollide(GODOWN, &block))
	{
		drawBlock(0);
		ULPY_INC;
		ualarm((useconds_t)(level * 100000), 0);
		drawBlock(1);
		refresh();
	}

	else
	{
		writeBlock(&block);
		newBlock();
		
		sighandler(SIGALRM);
	}
}


/**************************************************
*		"gameloop"
*
* Star of the show, where all the magic happens.
* For ever loop, the heads-up display (game info)
* is refreshed, the block is redrawn into its
* new posiition, and input is polled from the
* user.
*
* Input  : None
* Output : Returns an arbitrary exit code.
***************************************************/

int gameloop() 
{
	int ch = 'p';
	newBlock();
	drawBoard();
	
	// Draw game border.
	int borderLoop_i = 0;
	_borderLoop: 
		mvprintw( borderLoop_i++, 20, "|");
	if (borderLoop_i < 20) goto _borderLoop;
	
	// Initialize and begin alarm.
	signal(SIGALRM,sighandler); // Register signal handler
	ualarm((useconds_t)(level * 100000), 0);
	
_gameLoop:
	hud( &lines, &level);
	
	DRAWFRAME;
	ch = getchar();

	switch(ch) 
	{
		// Rotation case. Clones the current block, sends it off
		// to the external ASM file for processing, and then tests
		// the result for possible collisions. If it passes,
		// the current block is replaced with the result.
		case 'w':
			TETRINTEGER temp = block;
			temp = ASMrotateBlock(temp);
			
			if (!checkCollide(SPIN, &temp))
				block = temp;
		break;

		// Check collision to the left of the current position.
		case 'a':
			if (!checkCollide(GOLEFT, &block)) ULPX_DEC;
		break;

		// Check collision to the right of the current position.
		case 'd':
			if (!checkCollide(GORIGHT, &block)) ULPX_INC;
		break;

		// Skip the alarm, and move the block downward. 
		// Collision handled within alarm function.
		case 's':
			sighandler(SIGALRM);
		break;

		// new block (debug!)
		case 'b':  
			newBlock();
		break;
		
		default:
	}
	
	if (ch != 'e') goto _gameLoop;
	
	return 1;
}


/**************************************************
*		main method
***************************************************/

int main()
{
	// Begin curses
	initscr();
	curs_set(0);
	
	// Seed random number generation
	time_t t;
	srand((unsigned) time(&t));
	
	// If title() returns a non-zero value, the player has decided to start a game.
	// Otherwise, the player has decided to quit.
	if ( title() ) gameloop();

	// End curses mode
	endwin();
	curs_set(1);
	return 0;
}
