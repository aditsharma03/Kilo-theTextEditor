/****  header files  ****/ 
#include <asm-generic/ioctls.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/****  data  ****/ 

struct editorConfig
{
	int screen_rows;
	int screen_cols;
	struct termios original_termios;
};
struct editorConfig E;

/****  defines  ****/ 
#define CTRL_KEY(k) ((k) & 0x1f)

/****  terminal  ****/ 
void die( const char *s )
{
	write( STDOUT_FILENO, "\x1b[2J", 4 );
	write( STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if( tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original_termios) == -1 ) die("tcsetattr");
	return;
}

void enableRawMode()
{
	struct termios raw;
	
	if( tcgetattr(STDIN_FILENO, &E.original_termios) == -1 ) die("tcgetattr");
	atexit( disableRawMode);

	raw = E.original_termios;
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

int getWindowSize( int *rows, int *cols )
{
	struct winsize ws;
	if( ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws ) == -1 || ws.ws_col == 0 ) return -1;
	
	*rows = ws.ws_row;
	*cols = ws.ws_col;
	return 0;
}

/***** append buffer *****/ 

struct abuf{
	char *b;
	int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend( struct abuf *ab, const char *s, int len )
{
	char *new = realloc( ab->b, ab->len + len );

	if( new == NULL ) return;
	memcpy( &new[ab->len], s, len );
	ab->b = new;
	ab->len += len;
}

void abFree( struct abuf *ab )
{
	free( ab->b );
}

/****  input  ****/ 

void editorProcessKeypress()
{
	char c = editorReadKey();

	switch( c ){
		case CTRL_KEY('q'):
			write( STDOUT_FILENO, "\x1b[2J", 4 );
			write( STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}

/****  output  ****/ 

void editorDrawRows()
{
	int y;
	for( y=0; y<E.screen_rows; y++){
		write( STDOUT_FILENO, "~", 1);

		if( y < E.screen_rows-1 )
			write( STDOUT_FILENO, "\r\n", 2);
	}
}

void editorRefreshScreen()
{
	write( STDOUT_FILENO, "\x1b[2J", 4 );
	write( STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write( STDOUT_FILENO, "\x1b[H", 3);
}


/****  init  ****/ 

void initEditor()
{
	if( getWindowSize(&E.screen_rows, &E.screen_cols) == -1 ) die("getWindowSize");
	return;
}

int main (int argc, char *argv[])
{
	enableRawMode();
	initEditor();

	while(1){

		editorRefreshScreen();
		editorProcessKeypress();

	}

	return 0;
}

