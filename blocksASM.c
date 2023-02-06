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
// Global Variables /////////////////////////////////////////////////////////////////////

unsigned int ULPY = 1, ULPX = 1, score = 0, blockState = 0;
unsigned int board[20] = {0};
unsigned int block = 0;
int level = 9;

// Blocks
/* 00 01 10 11
00   
01  x  x  x
10     x
11

*/

unsigned int block_T = 0x4569;	// 0100010101101001b
unsigned int block_I = 0x4567;	// 0000000100100011b
unsigned int block_O = 0x569A;	// 0000010000010101b
unsigned int block_J = 0x0456;	// 0000010001010110b
unsigned int block_L = 0x4562;	// 0000000100100100b
unsigned int block_S = 0x4512; 	// 0001010101001000b
unsigned int block_Z = 0x0156; 	// 0000010001011001b

// Methods //////////////////////////////////////////////////////////////////////////////

void sighandler(int);

/*
void DEBUG() {
	move(0, 15);
	printw("x: %d", ULPX);
	move(3, 15);
	printw("y: %d ", ULPY);
	
	move(5, 15);
	printw("Score:%08d", score);
	
	move(0,30);
	printw("block state in memory:");
	for (int i = 0; i < 4; i++) {
		move(i,30);
		for (int j = 0; j < 4; j++)
			printw("%d", block[i][j]);
	}
	
	
	for (int i = 0; i < 20; i++) {
		move(i, 50);
		for (int j = 0; j < 10; j++) {
			printw("%d", board[i][j]);
		}
	}
}
*/

// Generate new tetromino


// Set tetromino in play /////////////////////////////////////////////////////////////

void newBlock() 
{	
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
	unsigned int temp = block;
	unsigned int mask = 0x3;		// 0000000000000011b
	unsigned int resultX, resultY;
	char out;
	
	if (x)
		out = '#';
	else
		out = ' ';
	
	for( int i = 0; i < 4; i++ )
	{
		resultX = temp & mask;
		temp = temp >> 2;
		resultY = temp & mask;
		temp = temp >> 2;
		mvprintw(ULPY + resultY, ULPX + resultX, "%c", out);
	}
	
	refresh();
}



// Check collisions. Return 0 if no, 1 if yes


int checkCollide(char dir, unsigned int *given) 
{	
	unsigned int temp = *given, bit = 0x0;
	unsigned int curr_X, curr_Y;
	unsigned int ofs_X = 0x0, ofs_Y = 0x0;
	unsigned int row, col;
	
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
		
		default: break;
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
		
		row = ULPY + curr_Y + ofs_Y;
		col = ULPX + curr_X + ofs_X;
		
		if ( row < 0 || row > 19 || col < 0 || col > 9 )
			return 1;
		
		bit = ( board[row] >> ( col - 1) ) & 0x1;
		
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

void rotateBlock( unsigned int *given ) 
{
	if ( *given == 0x159D ) 	*given = block_I;
	else if ( *given == block_I ) 	*given = 0x159D;
	else if ( *given == block_O);
	else
	{
		unsigned int current = 0x0;
		unsigned int accumulate = 0x0;
		
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
		
		if (!checkCollide(4, &accumulate))
		{
			*given = accumulate;
		}
	}
	
}

// Check for full line /////////////////////////////////////////////////////////////

int checkLine(int L) 
{	
	// 10x 1 digits, a full line
	if ( ( board[L] & 0x3FF ) != 0x3FF )
		return 0;
		
	for ( int i = L; i > 0; i--)
	{
		board[L] = board[L-1];
	}
	
	board[0] = 0x0;
	return 1;
}
// Write block to game board

void writeBlock() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[i][j]) {
				board[ULPY+i][ULPX+j] = 1;
				checkLine(ULPY+i);
			}
		}
	}
}

// Signal handler (move down) /////////////////////////////////////////////////////////////

void sighandler(int signum) 
{	
	drawBlock(0);
	
	if (!checkCollide(GODOWN, &block)) 
	{		
		ULPY++;
		ualarm((useconds_t)(level * 100000), 0);
	}
	
	else 
	{
		writeBlock();
		newBlock();
		
		ULPX = 4;
		ULPY = 0;
		
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
		move(i,10);
		printw("|");
	}
	
	newBlock();
	
	ULPY = 0;
	ULPX = 4;
	
	int ch = 'p';
	
	signal(SIGALRM,sighandler); // Register signal handler
	ualarm((useconds_t)(level * 100000), 0);
			
	while (ch != 'e') 
	{
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
					ULPX--;
			break;
			
			// check right:
			case 'd':	
				if (!checkCollide(GORIGHT, &block))
					ULPX++;
			break;
			
			//check down
			case 's':
				sighandler(SIGALRM);
			break;
			
			// new block (debug!)
			case 'b':	
				ULPX = 4;
				ULPY = 0;
				newBlock();
			break;
			
			default: break;
		}
	}
}


/*
// Main /////////////////////////////////////////////////////////////

int main(){
	time_t t;
	srand((unsigned) time(&t));	// Use current time to seed random number generation
	
	initscr();			// Begin curses
	gameloop();			// active game code
	endwin();			// End curses mode

	return 0;
}
*/

int main()
{
	initscr();			// Begin curses
	while (1)
	{
		gameloop();
	}
	
	endwin();			// End curses mode

	return 0;
}
