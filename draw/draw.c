#include "fonts_manager.h"
#include "disp_manager.h"
#include "encoding_manager.h"
#include "config.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "stdio.h"
#include <unistd.h>
#include <sys/mman.h>

#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>






//static PT_FontOpr gUserFreeTypeFile;
static PT_DispOpr gUserDisPlayMode;
static PT_EncodingOpr gUserEncodingOper;
static int g_dwFontSize; //字体大小
static int g_iFdTextFile; //文件指针
static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;
static unsigned char *g_pucLcdFirstPosAtFile;//第一个字符的位置
struct stat tStat;


int ShowOneFont ( PT_FontBitMap ptFontBitMap );

int OpenTextFile(char* pcfileName)
{
     g_iFdTextFile = open(pcfileName,O_RDONLY); //只读方式打开文件
     if(g_iFdTextFile < 0)
     {
         printf("open text file error\r\n");
		 return -1;
	 }

	 if(fstat(g_iFdTextFile, &tStat)) //获得统计信息
	 {
		printf("can't get fstat\n");
		return -1;
	 }

     //mmap把文件内容映射到一块内存
	 g_pucTextFileMem = (unsigned char* )mmap(NULL,tStat.st_size,PROT_READ, MAP_SHARED, g_iFdTextFile, 0);
    if (g_pucTextFileMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for text file\n");
		return -1;
	}

	 g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size; //得到文件结尾指针

	 gUserEncodingOper =  SelectEncodingOprForFile(g_pucTextFileMem); //根据文件编码得到合适的编码方式 
	 if (gUserEncodingOper)
	 {
		 g_pucLcdFirstPosAtFile = g_pucTextFileMem + gUserEncodingOper->iHeadLen; //得到第一个字符的位置
		 return 0;
	 }
	 else
	 {
		 return -1;
	 }

  
}





int SetTextAttr ( char *HzkFile,char* FreeTypeFile,char* DisplayMode, unsigned int Size )
{
	int iError,iRet=1;
    PT_FontOpr ptFontOpr,ptTmp;
	

	g_dwFontSize = Size;

    /********初始化显示结构体*********/
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


    ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead; //得到此编码方式能支持的点阵-字库

	
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

		printf("%s, %d\n", ptFontOpr->name, iError);

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
			DelFontOprFrmEncoding(gUserEncodingOper, ptFontOpr); //如果注册不成功 删除字库
		}
		ptFontOpr = ptTmp;
	}

    retunr iRet;

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
    unsigned char *pTextStart;
	
    tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = g_dwFontSize;
    
    pTextStart = g_pucLcdFirstPosAtFile;

    //一次只能处理一个字节
	while ( 1 )
	{
		iLen = gUserEncodingOper->GetCodeFrmBuf ( pTextStart, g_pucTextFileMemEnd, &dwCode );//取得编码
		if ( 0 == iLen )
		{
			/* 文件结束 */
			if ( bHasGetCode )//当显示过字符之后 打印文件结束
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

		pTextStart+=iLen;


		ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead;

		while ( ptFontOpr )
		{

           iError = ptFontOpr->GetFontBitmap ( dwCode, &tFontBitMap ); //取得字体位图

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
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX; //下一个显示字符位置
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


