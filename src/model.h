#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#define FILE_NAME "преступление и наказание.txt"

/* two modes:
	- classic mode
	- transfer by words */
typedef enum {
	classic,
	transfer
} mode_t;

/* move in text cases (arrows cases) */
typedef enum {
	right,
	left,
	up,
	down
} case_t;

typedef struct {
	int x;
	int y;
} pos_t;

/* struct to work with text */
typedef struct tagText {
	mode_t mode;        /* active mode */
	pos_t pos;          /* pos in text in symbols */
	LPSTR *clStrings;   /* array of clStrings in classic mode */
	LPSTR *trStrings;   /* array of clStrings in transfer mode */
	LPSTR buffer;       /* all text in buffer (useful to build clStrings in transfer mode) */
	int numClStrings;   /* num of classic lines */
	int numTrStrings;   /* num of lines in transfer mode */
	int curWidth;       /* current width in transfer mode */
	int maxWidth;       /* the longest string in classic mode */
	int bufLen;         /* buffer length */
} text_t;

int LoadText(text_t *text, char *fileName);
int OpenFileFunc(HWND hWnd, text_t *text, int width);
int BuildTrStrings(text_t *text, int width);
int SelectNOfLines(text_t text);
LPSTR *SelectStrings(text_t text);
int ClassToTrPos(text_t *text);
int TrToClassPos(text_t *text);

#endif // MODEL_H_INCLUDED
