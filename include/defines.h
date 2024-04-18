/************************************************************************************
	.########..########.########.####.##....##.########..######.
	.##.....##.##.......##........##..###...##.##.......##....##
	.##.....##.##.......##........##..####..##.##.......##......
	.##.....##.######...######....##..##.##.##.######....######.
	.##.....##.##.......##........##..##..####.##.............##
	.##.....##.##.......##........##..##...###.##.......##....##
	.########..########.##.......####.##....##.########..######.
	
	Written by Matthew Kenneth Peterson; ID 3719754; github "MKP157"
	
	@ Dr. Jong-Kyou Kim, CS 2253 - Final Project
	
	@ Last updated April 20, 2023
*************************************************************************************/

/**************************************************
*		Block definitions
***************************************************/

/* 00 01 10 11
00
01  x  x  x
10     x
11
*/

#define block_T 0x4569		// 0100010101101001b
#define block_I 0x4567		// 0000000100100011b
#define block_O 0x569A		// 0000010000010101b
#define block_J 0x456A		// 0000010001010110b
#define block_L 0x5679		// 0000000100100100b
#define block_S 0x679A 		// 0001010101001000b
#define block_Z 0x459A		// 0000010001011001b

/**************************************************
*		Anchor definitions
*
* Chars can represent 256 total values; we only need 
* up to 20 per y, and 10 per x. 2 chars together make 
* a short int. ULPX in lower half of the anchor so it 
* may be more easily decremented
*
***************************************************/

#define ANCHOR_RESET ANCHOR = 0x104; 
//y=0, x=4; 0000 0100 b => 0x04 h

#define ULPY_GET ((ANCHOR >> 8) & 0xFF) - 1
#define ULPY_INC ANCHOR += 0x100

#define ULPX_GET (ANCHOR & 0xFF) - 1
#define ULPX_INC ANCHOR += 0x1
#define ULPX_DEC ANCHOR -= 0x1

/**************************************************
*		Other definitions
***************************************************/

#define GOLEFT 0
#define GORIGHT 1
#define GODOWN 2
#define SPIN 3

#define DRAWFRAME drawBlock(0);drawBlock(1);refresh();
