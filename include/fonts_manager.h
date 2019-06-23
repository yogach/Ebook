
#ifndef _FONTS_MANAGER_H
#define _FONTS_MANAGER_H

typedef struct FontBitMap {
	int iXLeft;//最左坐标
	int iYTop;//最上坐标
	int iXMax;//x最大值
	int iYMax;//y最大值
	int iBpp;  //位图内的像素由位表示还是以字节表示
	int iPitch;   /* 对于单色位图, 两行象素之间的跨度 */
	int iCurOriginX;
	int iCurOriginY; //传入值当前源点位置
	int iNextOriginX;
	int iNextOriginY; //输出下一个源点位置
	unsigned char *pucBuffer; //点阵数据
}T_FontBitMap, *PT_FontBitMap;

typedef struct FontOpr {
	char *name;
	int (*FontInit)(char *pcFontFile, unsigned int dwFontSize); //字库初始化
	int (*GetFontBitmap)(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
	struct FontOpr *ptNext;
}T_FontOpr, *PT_FontOpr;


int RegisterFontOpr(PT_FontOpr ptFontOpr);
void ShowFontOpr(void);
int FontsInit(void);
int ASCIIInit(void);
int GBKInit(void);
int FreeTypeInit(void);
PT_FontOpr GetFontOpr(char *pcName);

#endif /* _FONTS_MANAGER_H */

