/* code by matt peterson. pls don't copy :( */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <signal.h>
#include <time.h>

#define GOLEFT 0
#define GORIGHT 1
#define GODOWN 2
#define SPIN 3

#define TETRINTEGER unsigned short int
#define BOARDTYPE unsigned short int

// Anchor ///////////////////////////////////////////////////////////////////////////////
// Chars can represent 256 total values; we only need up to 20 per y, and 10 per x.
// 2 chars together make a short int.
// ULPX in lower half of te anchor so it may be more easily decremented.

unsigned short int ANCHOR;

#define ANCHOR_RESET ANCHOR = 0x104; 
//y=0, x=4; 0000 0100 b => 0x04 h

#define ULPY_GET ((ANCHOR >> 8) & 0xFF) - 1
#define ULPY_INC ANCHOR += 0x100

#define ULPX_GET (ANCHOR & 0xFF) - 1
#define ULPX_INC ANCHOR += 0x1
#define ULPX_DEC ANCHOR -= 0x1


// Blocks
/* 00 01 10 11
00
01  x  x  x
10     x
11

*/
/* Anchor at top-left: */
#define block_T 0x4569		// 0100010101101001b
#define block_I 0x4567		// 0000000100100011b
#define block_O 0x569A		// 0000010000010101b
#define block_J 0x456A		// 0000010001010110b
#define block_L 0x5679		// 0000000100100100b
#define block_S 0x679A 		// 0001010101001000b
#define block_Z 0x459A		// 0000010001011001b

// Global Variables /////////////////////////////////////////////////////////////////////

BOARDTYPE 		board[20] = {0};
TETRINTEGER		block = 0;
const TETRINTEGER	possibleBlocks[7] = {block_T, block_I, block_O, block_J, block_L, block_S, block_Z};
unsigned char		level = 9;
//int 			score = 0;


// Methods //////////////////////////////////////////////////////////////////////////////

void sighandler(int);

// Set tetromino in play /////////////////////////////////////////////////////////////

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
_loop:
	// 0x3 is a 2-bit mask.
	resultX = temp & 0x3;
	temp >>= 2;
	resultY = temp & 0x3;
	temp >>= 2;
		
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
		printw("[]");
	
	else
		printw(" .");
		
	j--;
	if (j >= 0) goto _L2;
	i++;
	if (i < 20) goto _L1;

	refresh();
}


// Check collisions. Return 0 if no, 1 if yes


int checkCollide(int dir, TETRINTEGER *given)
{
	TETRINTEGER temp = *given;
	unsigned int bit = 0x0;
	int curr_X, curr_Y;
	int ofs_X = 0x0, ofs_Y = 0x0;
	int row, col;

	switch (dir)
	{
		// left
		case GOLEFT:
			ofs_X--;
		break;

		// right
		case GORIGHT:
			ofs_X++;
		break;

		// down
		case GODOWN:
			ofs_Y++;
		break;
	
		// we don't need to change the offset with rotation.
		default:
	}
	
	short int i = 0;
_Loop:
	curr_X = temp & 0x3;
	temp >>= 2;
	curr_Y = temp & 0x3;
	temp >>= 2;
	
	// shift all values of board right until focus bit is LSD;
	// ex: want bit #3
	// board[n] =   001010110 >> 3-1
	//		000010101
	//	&mask	000000001
	//	==	000000001 => true
	//
	// ex: want bit #6
	// board[n] =   001010110 >> 6-1
	//		000000010
	//	&mask	000000001
	//	==	000000000 => false
	
	row = ULPY_GET + curr_Y + ofs_Y;
	col = ULPX_GET + curr_X + ofs_X;
	
	/* Debug
	mvprintw(44+i*2,4,"Checked row:    %03hu", row);
	mvprintw(45+i*2,4,"Checked column: %03hu", col);
	*/
	
	if ( row < 0 || row > 19 || col < 0 || col > 9 )
		return 1;

	bit = ( board[row] >> ( 9 - col ) ) & 0x1;

	if (bit)
		return 1;
	
	i++;
	if (i < 4) goto _Loop;

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
	
	short int i = 0;

	_writeLoop:
		temp_col = ULPX_GET + (temp_block & 0x3);
		temp_block >>= 2;
	
		temp_row = ULPY_GET + (temp_block & 0x3);
		temp_block >>= 2;
		
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
	short int k;

	_checkLineLoop:
	
		if ( board[i] == 0x3FF )
		{
			k = i;
		
			_clearLineLoop:
				board[k] = board[k-1];
				k--;
				if (k > 1) goto _clearLineLoop;
		
			board[0] = 0;
		}
	
		i++;
		if (i < 20) goto _checkLineLoop;
		
	drawBoard();
	refresh();
}

// Signal handler (move down) /////////////////////////////////////////////////////////////

void sighandler(int signum)
{
	drawBlock(0);

	if (!checkCollide(GODOWN, &block))
	{
		ULPY_INC;
		ualarm((useconds_t)(level * 100000), 0);
	}

	else
	{
		drawBlock(1);
		writeBlock(&block);
		newBlock();
		
		sighandler(SIGALRM);
	}

	drawBlock(1);
	refresh();
}
// Game loop /////////////////////////////////////////////////////////////

int gameloop() 
{
	// draw board border
	int borderLoop_i = 0;

	_borderLoop:
		move(borderLoop_i,20);
		printw("|");
		borderLoop_i++;
		if (borderLoop_i < 20) goto _borderLoop;
	
	newBlock();
	drawBoard();
	
	int ch = 'p';

	signal(SIGALRM,sighandler); // Register signal handler
	ualarm((useconds_t)(level * 100000), 0);
	
_gameLoop:
	//DEBUG();
	refresh();
	drawBlock(1);
	refresh();
	ch = getchar();
	drawBlock(0);

	switch(ch) {
		// rotate
		case 'w':
			TETRINTEGER temp = block;
			temp = ASMrotateBlock(temp);
			
			if (!checkCollide(SPIN, &temp))
				block = temp;
			
		break;

		// check left:
		case 'a':
			if (!checkCollide(GOLEFT, &block))
				ULPX_DEC;
		break;

		// check right:
		case 'd':
			if (!checkCollide(GORIGHT, &block))
				ULPX_INC;
		break;

		//check down
		case 's':
			sighandler(SIGALRM);
		break;

		// new block (debug!)
		case 'b':
			newBlock();
		break;
		default: break;
	}
	
	if (ch != 'e') goto _gameLoop;
	
	gameOver:
	
	return 1;
}


int main()
{
	initscr();			// Begin curses
	curs_set(0);
	
	time_t t;
	srand((unsigned) time(&t));
	
	gameloop();

	endwin();			// End curses mode
	curs_set(1);
	return 0;
}
