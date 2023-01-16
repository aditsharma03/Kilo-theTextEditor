/*** header files ***/ 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

/*** data ***/ 
struct termios original_termios;

/*** terminal ***/ 
void die( const char *s )
{
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if( tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1 ) die("tcsetattr");
	return;
}

void enableRawMode()
{
	struct termios raw;
	
	if( tcgetattr(STDIN_FILENO, &original_termios) == -1 ) die("tcgetattr");
	atexit( disableRawMode);

	raw = original_termios;
	raw.c_lflag &= ~( ECHO | ICANON | ISIG | IEXTEN );
	raw.c_iflag &= ~( IXON | ICRNL );
	raw.c_oflag &= ~( OPOST );

	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if( tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw ) == -1 ) die("tcsetattr");
}

/*** init ***/ 
int main (int argc, char *argv[])
{
	enableRawMode();

	while(1){

		char c = '\0';
		if( read( STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN ) die("read");
		if( iscntrl(c) )
			printf("%d\r\n", c );
		else
			printf("%d %c\r\n", c, c );

		if( c == 'q' ) break;
	}

	return 0;
}

