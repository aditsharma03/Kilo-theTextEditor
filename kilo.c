#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios original_termios;

void disableRawMode()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
	return;
}

void enableRawMode()
{
	struct termios raw;
	
	tcgetattr(STDIN_FILENO, &original_termios);
	atexit( disableRawMode);

	raw = original_termios;
	raw.c_lflag &= ~( ECHO | ICANON );
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw );
}

int main (int argc, char *argv[])
{
	enableRawMode();

	char c;
	while ( read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
		if( iscntrl(c) )
			printf("%d\n", c );
		else
			printf("%d %c\n", c, c );
	}
	return 0;
}
