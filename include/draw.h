
#ifndef _DRAW_H
#define _DRAW_H
#include "fonts_manager.h"

typedef struct PageDesc
{
  int PageNum;
  unsigned char *CurPageStart;
  unsigned char *NextPageStart;
  struct PageDesc *prePageDesc;
  struct PageDesc *nextPageDesc; 
}T_PageDesc,*PT_PageDesc;



int SetTextAttr ( char *HzkFile,char* DisplayMode, unsigned int Size );
int ShowOnePage ( unsigned char* str );
int OpenTextFile(char* pcfileName);
int ShowOneFont ( PT_FontBitMap ptFontBitMap );
int ShowNextPage(void);
int ShowPrePage ( void );


#endif /* _DRAW_H */

