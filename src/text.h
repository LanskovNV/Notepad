#ifndef _TEXT_H
#define _TEXT_H
/** input: all text in buffer (string)
    output: dword num of '\n' in buffer
    desc: This func count num of lines in text */
DWORD GetNumLines(LPSTR text);

/** input: char symbol
    output: true or false
    desc: find out is input char a space or not */
BOOL IsSpace(char c);

#endif
