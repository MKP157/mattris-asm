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


// Anchor ///////////////////////////////////////////////////////////////////////////////
// Chars can represent 256 total values; we only need up to 20 per y, and 10 per x.
// 2 chars together make a short int.
// ULPX in lower half of te anchor so it may be more easily decremented.

unsigned short int ANCHOR;

#define ANCHOR_RESET ANCHOR = 0x5; 
//y=0, x=4; 0000 0100 b => 0x04 h

#define ULPY_GET ((ANCHOR >> 8) & 0xFF) - 1
#define ULPY_INC ANCHOR += 0x100

#define ULPX_GET (ANCHOR & 0xFF) - 1
#define ULPX_INC ANCHOR += 0x1
#define ULPX_DEC ANCHOR -= 0x1


// Global Variables /////////////////////////////////////////////////////////////////////
#define TETRINTEGER unsigned short int
#define BOARDTYPE unsigned short int

BOARDTYPE 		board[20] = {0};
TETRINTEGER		block = 0;
unsigned char		level = 9;
//int 			score = 0;

// Blocks
/* 00 01 10 11
00
01  x  x  x
10     x
11

*/

TETRINTEGER block_T = 0x4569;	// 0100010101101001b
TETRINTEGER block_I = 0x4567;	// 0000000100100011b
TETRINTEGER block_O = 0x569A;	// 0000010000010101b
TETRINTEGER block_J = 0x0456;	// 0000010001010110b
TETRINTEGER block_L = 0x4562;	// 0000000100100100b
TETRINTEGER block_S = 0x4512; 	// 0001010101001000b
TETRINTEGER block_Z = 0x0156; 	// 0000010001011001b

// Methods //////////////////////////////////////////////////////////////////////////////

void sighandler(int);


void DEBUG()
{
	//mvprintw(40,4,"ULPY: %03hu", ULPY_GET);
	//mvprintw(41,4,"ULPX: %03hu", ULPX_GET);
	
	for (int i = 0; i < 20; i++)
	{
		for (int j = 16; j >= 0; j--)
		{
			mvprintw(i, 30 - j, "%c", ((board[i] >> j) & 0x1) + '0');
			refresh();
		}
		
		mvprintw(i, 50, "%04x", board[i]);
	}
}



// Set tetromino in play /////////////////////////////////////////////////////////////

void newBlock()
{
	ANCHOR_RESET;
	
	int nextBlock = rand() % 7;

	switch(nextBlock)
	{
			case 0:	// T
			block = block_T;
			break;

			case 1:	// I
			block = block_I;
			break;

			case 2: // O
			block = block_O;
			break;

			case 3:  // J
			block = block_J;
			break;

			case 4: // L
			block = block_L;
			break;

			case 5: // S
			block = block_S;
			break;

			case 6: // Z
			block = block_Z;
			break;

			default: break;
	}
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

	for( int i = 0; i < 4; i++ )
	{
		// 0x3 is a 2-bit mask.
		resultX = temp & 0x3;
		temp = temp >> 2;
		resultY = temp & 0x3;
		temp = temp >> 2;
		
		mvprintw(ULPY_GET + resultY, (ULPX_GET + resultX) * 2, "%s", out);
	}

	refresh();
}

// draw board

void drawBoard()
{
	int temp1, temp2;

	for (int i = 0; i < 20; i++)
	{
		move(i, 0);
		temp1 = board[i];

		for (int j = 9; j >= 0; j--)
		{
			temp2 = (board[i] >> j);

			if (temp2 & 0x1)
			{
				printw("[]");
			}

			else
			{
				printw(" .");
			}
		}
	}

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
		break;
	}

	for( int i = 0; i < 4; i++ )
	{
		curr_X = temp & 0x3;
		temp = temp >> 2;
		curr_Y = temp & 0x3;
		temp = temp >> 2;

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
		
		mvprintw(44+i*2,4,"Checked row:    %03hu", row);
		mvprintw(45+i*2,4,"Checked column: %03hu", col);
		
		if ( row < 0 || row > 19 || col < 0 || col > 9 )
			return 1;

		bit = ( board[row] >> ( 9 - col ) ) & 0x1;

		if (bit)
			return 1;
	}

	return 0;
}

// rotate block both in theory and on game board
// Pattern:
//	Outer:
//		0000 -> 1000 -> 1010 -> 0010
//		0x0	0x8	0xA	0x2
//
//	Middle:
//		0101 -> 0101
//		0x5	0x5
//
//	Outer:
//		0001 -> 0100 -> 1001 -> 0110
//		0x1	0x4	0x9	0x6

void rotateBlock( TETRINTEGER *given )
{
	if ( *given == 0x159D ) 	*given = block_I;
	else if ( *given == block_I ) 	*given = 0x159D;
	else if ( *given == block_O);

	else
	{
		TETRINTEGER current = 0x0;
		TETRINTEGER accumulate = 0x0;

		for( int i = 0; i < 16; i += 4 )
		{
			unsigned int temp = *given;
			current = (temp & (0xF << i)) >> i;

			switch (current)
			{
				case 0x0:	current = 0x8;	break;
				case 0x8:	current = 0xA;	break;
				case 0xA:	current = 0x2;	break;
				case 0x2:	current = 0x0;	break;

				case 0x1:	current = 0x4;	break;
				case 0x4:	current = 0x9; 	break;
				case 0x9:	current = 0x6; 	break;
				case 0x6:	current = 0x1;	break;

				default: break;
			}

			accumulate = (accumulate << 4) | current;
		}

		if (!checkCollide(SPIN, &accumulate))
		{
			*given = accumulate;
		}
	}

}

// Check for full line /////////////////////////////////////////////////////////////

void checkLine(int line) {
	
	if (board[line] == 0x3FF) { //if row full, compare = 1 or TRUE
	
		for (int i = line; i > 0; i--) 
		{
			board[i] = board[i-1];
		}
	}
}

// Write block to game board

void writeBlock(TETRINTEGER *given)
{
	TETRINTEGER temp_block = *given;
	int temp_rows[4] = {0,0,0,0};
	unsigned int temp_col, temp_row;

	for ( int i = 0; i < 4; i++ )
	{
		temp_col = ULPX_GET + (temp_block & 0x3);
		temp_block = temp_block >> 2;

		temp_row = ULPY_GET + (temp_block & 0x3);
		temp_block = temp_block >> 2;

		board[temp_row] = board[temp_row] + (0x1 << (9-temp_col));
	}
	
	// Clearing lines
	
	for ( int j = 0; j < 20; j++ )
	{
		if ( board[j] == 0x3FF )
		{
			for ( int k = j; k > 1; k--)
			{
				board[k] = board[k-1];
			}
			
			board[0] = 0;
		}
	}
		
	DEBUG();
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
		writeBlock(&block);
		drawBoard();
		
		newBlock();

		sighandler(SIGALRM);
	}

	drawBlock(1);
	refresh();
}
// Game loop /////////////////////////////////////////////////////////////

void gameloop() {

	// draw blank board
	for ( int i = 0; i < 20; i++ )
	{
		move(i,20);
		printw("|");
	}

	newBlock();

	int ch = 'p';

	signal(SIGALRM,sighandler); // Register signal handler
	ualarm((useconds_t)(level * 100000), 0);
	
	while (ch != 'e')
	{
		//DEBUG();
		refresh();
		drawBoard();
		drawBlock(1);
		refresh();
		ch = getchar();
		drawBlock(0);

		switch(ch) {
			// rotate
			case 'w':
				rotateBlock(&block);
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
	}
}


int main()
{
	initscr();			// Begin curses
	curs_set(0);
	gameloop();

	endwin();			// End curses mode
	curs_set(1);
	return 0;
}
