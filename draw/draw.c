#include "fonts_manager.h"
#include "stdio.h"
#include "disp_manager.h"
#include <string.h>
#include "encoding_manager.h"
#include "config.h"


static PT_FontOpr gUserFreeTypeFile;
static PT_DispOpr gUserDisPlayMode;
static PT_EncodingOpr gUserEncodingOper;
static int g_dwFontSize; //字体大小


int ShowOneFont ( PT_FontBitMap ptFontBitMap );

int GetFileEncodingType(char* fileStart)
{


}



int SetTextAttr ( char *HzkFile,char* FreeTypeFile,char* DisplayMode, unsigned int Size )
{
	int iError,iRet;
    PT_FontOpr ptFontOpr,ptTmp;
	//ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;

	g_dwFontSize = Size;

    /***********************/
	gUserDisPlayMode =  GetDispOpr ( DisplayMode ); //根据显示结构体的名字得到对应结构体
	if ( NULL == gUserDisPlayMode )
	{
		printf ( "GetDispOpr error\r\n" );
		return -1;
	}
	iError =  gUserDisPlayMode->DeviceInit();
	if ( iError == -1 )
	{
		printf ( "Display init error\r\n" );
	}
	/***********************/

    //根据文件的编码方式得到对应结构体 
	gUserEncodingOper = SelectEncodingOprForFile ( "ascii" );
	if ( NULL == gUserEncodingOper )
	{
		printf ( " SelectEncodingOpr error\r\n" );
		return -1;
	}

    ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead; //得到此编码方式能支持的点阵

	
	while (ptFontOpr)
	{
		if (strcmp(ptFontOpr->name, "ascii") == 0)
		{
			iError = ptFontOpr->FontInit(NULL, Size);
		}
		else if (strcmp(ptFontOpr->name, "gbk") == 0)
		{
			iError = ptFontOpr->FontInit(HzkFile, Size);
		}
		else
		{
			iError = ptFontOpr->FontInit(FreeTypeFile, Size);
		}

		DBG_PRINTF("%s, %d\n", ptFontOpr->name, iError);

		ptTmp = ptFontOpr->ptNext;

		if (iError == 0)
		{
			/* 比如对于ascii编码的文件, 可能用ascii字体也可能用gbk字体, 
			 * 所以只要有一个FontInit成功, SetTextDetail最终就返回成功
			 */
			iRet = 0;
		}
		else
		{
			DelFontOprFrmEncoding(gUserEncodingOper, ptFontOpr);
		}
		ptFontOpr = ptTmp;
	}



}


int ShowOnePage ( unsigned char* str )
{
	int bHasNotClrSceen = 1;
	int iError;
	int iLen;
	unsigned int dwCode;
	T_FontBitMap tFontBitMap;
	int bHasGetCode = 0;
	PT_FontOpr ptFontOpr;

	
    tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = g_dwFontSize;


    //一次只能处理一个字节
	while ( 1 )
	{
		iLen = gUserEncodingOper->GetCodeFrmBuf ( str, str+4, &dwCode );//取得
		if ( 0 == iLen )
		{
			/* 文件结束 */
			if ( !bHasGetCode )
			{
			    printf("file end\r\n");
				return -1;
			}
			else
			{
			    printf("GetCodeFrmBuf error\r\n");
				return 0;
			}
		}
		bHasGetCode = 1;

		str+=iLen;

		
		while ( ptFontOpr )
		{

            ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead;
		
			iError = gUserFreeTypeFile->GetFontBitmap ( dwCode, &tFontBitMap ); //取得字体位图

			if ( iError != -1 )
			{
				if ( bHasNotClrSceen )
				{
					/* 首先清屏 */
					gUserDisPlayMode->CleanScreen ( COLOR_BACKGROUND );
					bHasNotClrSceen = 0;
				}

				ShowOneFont ( &tFontBitMap );
			}
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX;
			tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;

            ptFontOpr= ptFontOpr->ptNext;
			
		}
	}

}



int ShowOneFont ( PT_FontBitMap ptFontBitMap )
{
	int y;
	int x;
	unsigned char ucByte = 0;
	int i = 0;
	int bit;


	if ( ptFontBitMap->iBpp == 1 )
	{
		for ( y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++ )
		{
			i = ( y - ptFontBitMap->iYTop ) * ptFontBitMap->iPitch;
			for ( x = ptFontBitMap->iXLeft, bit = 7; x < ptFontBitMap->iXMax; x++ )
			{
				if ( bit == 7 )
				{
					ucByte = ptFontBitMap->pucBuffer[i++];
				}

				if ( ucByte & ( 1<<bit ) ) //描点
				{
					gUserDisPlayMode->ShowPixel ( x, y, COLOR_FOREGROUND );
				}
				else
				{
					/* 使用背景色, 不用描画 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* 黑 */
				}
				bit--;
				if ( bit == -1 )
				{
					bit = 7;
				}
			}
		}


	}
	else if ( ptFontBitMap->iBpp == 8 )
	{
		for ( y = ptFontBitMap->iYTop; y < ptFontBitMap->iYMax; y++ )
			for ( x = ptFontBitMap->iXLeft; x < ptFontBitMap->iXMax; x++ )
			{
				//g_ptDispOpr->ShowPixel(x, y, ptFontBitMap->pucBuffer[i++]);
				if ( ptFontBitMap->pucBuffer[i++] )
				{
					gUserDisPlayMode->ShowPixel ( x, y, COLOR_FOREGROUND );
				}
			}


	}
	else
	{
		printf ( "ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp );
	}




}


