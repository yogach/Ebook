
#ifndef _DRAW_H
#define _DRAW_H
#include "fonts_manager.h"



int SetTextAttr ( char *HzkFile,char* DisplayMode, unsigned int Size );
int ShowOnePage ( unsigned char* str );
int OpenTextFile(char* pcfileName);
int ShowOneFont ( PT_FontBitMap ptFontBitMap );
int ShowNextPage(void);


#endif /* _DRAW_H */

