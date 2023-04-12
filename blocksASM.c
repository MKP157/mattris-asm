/* code by matt peterson. pls don't copy :( */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include "./defines.h"
#include "./title.c"

/**************************************************
*		Global definitions
***************************************************/
unsigned short int 	ANCHOR;
unsigned int		level = 9;
unsigned int		lines = 0;

#define BOARDTYPE unsigned short int
BOARDTYPE 		board[20] = {0};
volatile BOARDTYPE 	*rowPointer = &board[0];

#define TETRINTEGER unsigned short int
TETRINTEGER		block = 0;
const TETRINTEGER	possibleBlocks[7] = {block_T, block_I, block_O, block_J, block_L, block_S, block_Z};

void sighandler(int);

/**************************************************
***************************************************/
void newBlock()
{
	ANCHOR_RESET;
	block = possibleBlocks[ rand() % 7 ];
}

// draw active block. parameter x: 0=erase, 1=draw

void drawBlock(int x)
{
	TETRINTEGER temp = block;
	unsigned int resultX, resultY;
	char* out;

	if (x)
		out = "[]";
	else
		out = " .";
	
	short int i = 0;
	
	mvprintw( 9, 30, "< * BLOCK DRAW DEBUG * >");
	
	_loop:
		
		/*// 0x3 is a 2-bit mask.
		resultX = temp & 0x3;
		temp >>= 2;
		resultY = temp & 0x3;
		temp >>= 2;*/
		
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
			
		// debug:
		mvprintw( i+10, 30, "%d,%d : %08d", resultX, resultY, temp );
		
		mvprintw(ULPY_GET + resultY, (ULPX_GET + resultX) * 2, "%s", out);
		
	i++;
	if (i < 4) goto _loop;

	refresh();
}

// draw board

void drawBoard()
{
	int temp1, temp2;
	short int i = 0, j;
	_L1:
		move(i, 0);
		temp1 = board[i];
		j = 9;
		
		_L2:
			temp2 = (board[i] >> j);

			if (temp2 & 0x1)
			{
				attron(COLOR_PAIR( i % 7 ));
				printw("[]");
				attron(COLOR_PAIR(7));
			}
			else
				printw(" .");
		
			j--;
		if (j >= 0) goto _L2;
	
		i++;
	if (i < 20) goto _L1;

	refresh();
}


// Check collisions. Return 0 if no, 1 if yes /////////////////////////////////


int checkCollide(int dir, TETRINTEGER *given)
{
	drawBlock(0);
	
	TETRINTEGER temp = *given;
	int ux = ULPX_GET;
	int uy = ULPY_GET;
	int bit = 0x0, result = 0;
	// Variables unnecessary due to ASM
	// int curr_X, curr_Y;
	int ofs_X = 0, ofs_Y = 0;
	int row, col;
	
	switch (dir)
	{
		// left
		case GOLEFT:
			//ofs_X--;
			ofs_X = -1;
		break;

		// right
		case GORIGHT:
			ofs_X = 1;
		break;

		// down
		case GODOWN:
			ofs_Y = 1;
		break;
	
		// we don't need to change the offset with rotation.
		default:
	}
	
	short int i = 0;
	
	mvprintw(15, 30, "< * COLLISION DEBUG * >");
	
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
		
		mvprintw( i+16, 30, "result:%d, temp:%d", result, temp);
		refresh();
		
		if (result) return 1;
		
		i++;
	if (i < 4 && !result) goto _Loop;

	return 0;
}

// rotate block both in theory and on game board

extern TETRINTEGER ASMrotateBlock ( TETRINTEGER given );

// Write block to game board

void writeBlock(TETRINTEGER *given)
{
	TETRINTEGER temp_block = *given;
	int temp_rows[4] = {0,0,0,0};
	unsigned int temp_col, temp_row;
	int i = 0;

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
		// we avoid segmentation faults.
		if ( board[temp_row] & (0x1 << (9-temp_col)) )
		{
			endwin();
			
			printf("   ___   _   __  __ ___    _____   _____ ___ \n");
			printf("  / __| /_\\ |  \\/  | __|  / _ \\ \\ / / __| _ \\\n");
			printf(" | (_ |/ _ \\| |\\/| | _|  | (_) \\ V /| _||   /\n");
			printf("  \\___/_/ \\_\\_|  |_|___|  \\___/ \\_/ |___|_|_\\\n");
			printf("... you stacked too high! Better luck next time.\n");
			
			curs_set(1);
			exit(1);
		}
		
		board[temp_row] = board[temp_row] + (0x1 << (9-temp_col));
		
		i++;
	
	if (i < 4) goto _writeLoop;
	
	// Clearing lines
	i = 0;
	
	int k;

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
		}
	
		i++;
		if (i < 20) goto _checkLineLoop;
		
	drawBoard();
	refresh();
}

// Signal handler (move down) /////////////////////////////////////////////

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
		
		if ( !(lines % 10) && (lines != 0) && (level > 1) ) level--;
		
		sighandler(SIGALRM);
	}
}
// Game loop /////////////////////////////////////////////////////////////

int gameloop() 
{
	// Draw game border.
	int borderLoop_i = 0;
	
	_borderLoop: 
		mvprintw( borderLoop_i++, 20, "|");
	if (borderLoop_i < 20) goto _borderLoop;
	
	drawBoard();
	
	// Initialize and begin alarm.
	signal(SIGALRM,sighandler); // Register signal handler
	ualarm((useconds_t)(level * 100000), 0);
	
	newBlock();
	int ch = 'p';

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


int main()
{
	initscr();			// Begin curses
	curs_set(0);
	
	time_t t;
	srand((unsigned) time(&t));
	
	if ( title() ) gameloop();

	endwin();			// End curses mode
	curs_set(1);
	return 0;
}
