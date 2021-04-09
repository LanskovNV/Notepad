#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#define FILE_NAME "преступление и наказание.txt"

/* two modes:
	- classic mode
	- transfer by words */
typedef enum {
	classic,
	transfer
} my_mode_t;

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

typedef struct {
	LPSTR string;
	int strLen;
} string_t;

/* struct to work with text */
typedef struct tagText {
	my_mode_t mode;     /* active mode */
	pos_t pos;          /* pos in text in symbols */
	string_t *classic;
	string_t *transfer;
	LPSTR buffer;       /* all text in buffer (useful to build clStrings in transfer mode) */
	int numClStrings;   /* num of classic lines */
	int numTrStrings;   /* num of lines in transfer mode */
	int curWidth;       /* current width in transfer mode */
	int maxWidth;       /* the longest string in classic mode */
	int bufLen;         /* buffer length */
} text_t;

/** input: text struct, new file name
    output: int (0 - ok, else - error)
    desc: This func open text and init fields of text struct */
int LoadText(text_t *text, char *fileName);

/** input: hwnd, text struct, current width
    output: int (0 - ok, else - error)
    desc: this func support open file menu - open file manager and load new text */
int OpenFileFunc(HWND hWnd, text_t *text, int width);

/** input: text struct, current screen width
    output: int (0 - ok, else - error)
    desc: generate mas of string_t elements to display text in transfer mode */
int BuildTrStrings(text_t *text, int width);

/** input: text struct
    output: current num of strings
    desc: select current strings number by active mode */
int SelectNOfLines(text_t text);

/** input: text struct
    output: mas of string_t elements
    desc: return current "text view" by active mode*/
string_t *SelectStrings(text_t text);

/** input: text struct
    output: int (0 - ok, else - error)
    desc: count pos in transfer mode by classic */
int ClassToTrPos(text_t *text);

/** input: text struct
    output: int (0 - ok, else - error)
    desc: count pos in classic mode by transfer */
int TrToClassPos(text_t *text);

/** input: text struct
    output: no
    desc: free memory from text struct */
void ClearText(text_t *text);
#endif // MODEL_H_INCLUDED
