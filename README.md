# mattris-asm
Revising and rewriting _mattris'_ C code, and when the time comes, translating it to x86_64 ASM and beyond.

# Preparing for ASM
In order to be ready to program _mattris_ for x86_64 assembly, there will be a number of changes in order to make its original C code as analogous to an assembly counterpart as possible. 

So far, these changes include:

- The TetrInteger; a binary representation of a given block, which crams eight 2-bit coordinates (y,x,y,x,y,x,y,x) into a short int (16 digits), so that one block may fit into a single register. This data compression is more of a future-proofing measure, as I intend to some day port this to an 8-bit micro from the 1980s.
- All associated block-operations have been modified to account for the TetrInteger's structure. For example, accessing an individual block coordinate requires bit-shifting, rather than accessing a distinct array value.
- All instances of conditional loops have been rewritten into their respective 'goto' loop counterparts, to become familar with assembly's conditional jumps. C's 'goto' functionality is identical to that of assembly's.

# ASM Rewrites
- "rotate.s" : The block rotation function, completely rewritten into assembly and linked to the C program. 

# Compiling the C source code
I recommend compiling the code within the latest release.

To run the C version of _mattris_, you must be running a Linux or UNIX-like operating system. This is because the code makes extensive use of the 'ncurses' library, which directly manipulates terminal graphics. As far as I am aware, 'ncurses' has no actively maintained Windows terminal implementation. I have used 'ncurses' successfully on Debian Linux distrobutions, as well as Apple's macOS through the Homebrew package manager (https://brew.sh/). To compile, I use GCC, though I presume you may use others.

**To install 'ncurses' on Debian:**
`sudo apt-get install libncurses5-dev libncursesw5-dev `

**macOS with Homebrew:** `brew install ncurses`

**To compile:**
1. Using your terminal, navigate to where you have stored the blocksASM.c source file.

2. Compile with `gcc -o blocksASM blocksASM.c rotate.s -lncurses` .

3. Run with `./blocksASM` .
