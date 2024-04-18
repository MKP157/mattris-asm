/*
 *  Very simple methods for drawing more complex graphics in larger batches.
 *  No real commenting here, as I felt at these methods were fairly self-descriptive.
 */


int title ()
{
	
	int i = 0;
	mvprintw(i++,1, " ___      ___       __  ___________  ___________  _______    __      ________  " );
	mvprintw(i++,1, "|\"  \\    /\"  |     /\"\"\\(\"     _   \")(\"     _   \")/\"      \\  |\" \\    /\"       ) " );
	mvprintw(i++,1, " \\   \\  //   |    /    \\)__/  \\\\__/  )__/  \\\\__/|:        | ||  |  (:   \\___/  " );
	mvprintw(i++,1, "|: \\.        |  //  __'  \\ |.  |        |.  |    //      /  |.  |    __/  \\\\   " );
	mvprintw(i++,1, "|.  \\    /:  | /   /  \\\\  \\\\:  |        \\:  |   |:  __   \\  /\\  |\\  /\" \\   :)  " );
	mvprintw(i++,1, "|___|\\__/|___|(___/    \\___)\\__|         \\__|   |__|  \\___)(__\\_|_)(_______/   " );
	
	start_color();
	attron(A_BOLD);
	
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_CYAN, COLOR_BLACK);
	init_pair(5, COLOR_BLUE, COLOR_BLACK);
	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(7, COLOR_WHITE, COLOR_BLACK);
	init_pair(8, COLOR_WHITE, COLOR_BLACK);
	init_pair(9, COLOR_WHITE, COLOR_BLACK);
	
	int j = 15;
	attron(COLOR_PAIR(1)); 	mvprintw(i++,j, ".............................................." );
	attron(COLOR_PAIR(1)); 	mvprintw(i++,j, "....########......#########...####......####.." );
	attron(COLOR_PAIR(2)); 	mvprintw(i++,j, "...####..####...#####.........######..######.." );
	attron(COLOR_PAIR(3)); 	mvprintw(i++,j, "..####....####....########....##############.." );
	attron(COLOR_PAIR(4)); 	mvprintw(i++,j, "..############.........#####..####.####.####.." );
	attron(COLOR_PAIR(5)); 	mvprintw(i++,j, "..###......###...#########....####..##..####.." );
	attron(COLOR_PAIR(6)); 	mvprintw(i++,j, ".............................................." );
				
	attron(COLOR_PAIR(7));	mvprintw( i+5,j, "    >>      Press \"y\" to play.     <<" );
				
				mvprintw( i+7,j, "    >>  Press any other key to quit. <<" );
				
	attron(COLOR_PAIR(6));	mvprintw(i+10,j, "    By Matthew Kenneth Peterson, 2022/2023" );
				mvprintw(i+11,j-10, "ASCII art by Patrick Gillespie ( http://patorjk.com/software/taag/ ) " );
	
	attron(COLOR_PAIR(7));
	attroff(A_BOLD);
	
	refresh();
	
	char test = getch();
	
	clear();
	
	return (test == 'y');
}

void hud ( int* lines, int* level )
{
	attron( COLOR_PAIR(*level) );
	mvprintw( 1, 30, "============" );
	mvprintw( 2, 30, " Level: %03d", 9-(*level) );
	mvprintw( 3, 30, " Lines: %03d", (*lines) );
	mvprintw( 4, 30, "============" );
	attron( COLOR_PAIR(7) );
	
	refresh();
}

void gameOver ()
{
	printf("   ___   _   __  __ ___    _____   _____ ___ \n");
	printf("  / __| /_\\ |  \\/  | __|  / _ \\ \\ / / __| _ \\\n");
	printf(" | (_ |/ _ \\| |\\/| | _|  | (_) \\ V /| _||   /\n");
	printf("  \\___/_/ \\_\\_|  |_|___|  \\___/ \\_/ |___|_|_\\\n");
	printf("... you stacked too high! Better luck next time.\n");
}
