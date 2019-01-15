#pragma once
#ifndef _STRUCTURES_H
#define _STRUCTURES_H

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

/* struct to display text */
typedef struct tagView {       
	pos_t client;       /* screen size */
	pos_t charSize;
	pos_t caret;        /* current pos on screen */
	int iMaxWidth;      /* max Hscroll length */
	int iVscrollMax;
	int iVscrollPos;
	int iHscrollMax;
	int iHscrollPos;
	int iSelection;    /* active menu flag (mode) */
} view_t;

/* struct to work with text */
typedef struct tagtext_t {
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

#endif