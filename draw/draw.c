#include "fonts_manager.h"
#include "stdio.h"
#include "disp_manager.h"
#include <string.h>


static PT_FontOpr gUserFreeTypeFile;
static PT_DispOpr gUserDisPlayMode;
static PT_EncodingOpr gUserEncodingOper;
static int g_dwFontSize; //字体大小


int SetTextAttr ( char HzkFile,char* FreeTypeFile,char* DisplayMode, unsigned int Size )
{
	int iError;

	//ptFontOpr = g_ptEncodingOprForFile->ptFontOprSupportedHead;

	g_dwFontSize = Size;


	gUserFreeTypeFile = GetFontOpr ( FreeTypeFile ); //
	if ( NULL == gUserFreeTypeFile )
	{
		printf ( "GetFontOpr error\r\n" );

	}

	//此处为了支持不同的字库文件
	if ( strcmp ( gUserFreeTypeFile->name, "ascii" ) == 0 )
	{
		iError = gUserFreeTypeFile->FontInit ( NULL, Size );
	}
	else if ( strcmp ( gUserFreeTypeFile->name, "gbk" ) == 0 )
	{
		iError = gUserFreeTypeFile->FontInit ( HzkFile, Size );
	}
	else
	{
		iError = gUserFreeTypeFile->FontInit ( FreeTypeFile, Size );
	}

	if ( iError == -1 )
	{
		printf ( "FontInit error\r\n" );
	}


	gUserDisPlayMode =  GetDispOpr ( DisplayMode );
	if ( NULL == gUserDisPlayMode )
	{
		printf ( "GetDispOpr error\r\n" );
		return -1;
	}

	iError =  gUserDisPlayMode.DeviceInit();
	if ( iError == -1 )
	{
		printf ( "Display init error\r\n" );
	}

	gUserEncodingOper = SelectEncodingOprForFile ( "ascii" );
	if ( NULL == gUserEncodingOper )
	{
		printf ( " SelectEncodingOpr error\r\n" );
		return -1;
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
				return -1;
			}
			else
			{
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
					g_ptDispOpr->ShowPixel ( x, y, COLOR_FOREGROUND );
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
					g_ptDispOpr->ShowPixel ( x, y, COLOR_FOREGROUND );
				}
			}


	}
	else
	{
		printf ( "ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp );
	}




}


