/*** header files ***/ 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

/*** data ***/ 
struct termios original_termios;

/*** defines ***/ 
#define CTRL_KEY(k) ((k) & 0x1f)

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

char editorReadKey()
{
	int nread;
	char c;
	while( (nread = read(STDIN_FILENO, &c, 1)) != 1 ){
		if( nread == -1 && errno != EAGAIN ) die("read");
	}
	return c;
}

/*** input ***/ 

void editorProcessKeypress()
{
	char c = editorReadKey();

	switch( c ){
		case CTRL_KEY('q'):
			exit(0);
			break;
	}
}


/*** init ***/ 
int main (int argc, char *argv[])
{
	enableRawMode();

	while(1){

		editorProcessKeypress();

	}

	return 0;
}

