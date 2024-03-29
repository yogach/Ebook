#include "fonts_manager.h"
#include "disp_manager.h"
#include "encoding_manager.h"
#include "config.h"
#include "draw.h"


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
static unsigned char* g_pucTextFileMem;  //文本文件起始位置
static unsigned char* g_pucTextFileMemEnd;//文本文件结束位置
static unsigned char* g_pucLcdFirstPosAtFile;//第一个字符的位置
static struct stat tStat;
static unsigned char FirstDis;

static PT_PageDesc g_ptCurPage; //当前页面
static PT_PageDesc g_ptPageHead;

//static unsigned char* g_pucCurPageStart; //当前显示页面的起始地址
static unsigned char* g_pucNextPageStart;//当前显示页面的结束地址


//换行
int LineFeed ( int lcdY )
{
	lcdY+=g_dwFontSize;
	if ( lcdY >gUserDisPlayMode->iYres )
	{
		return 0;
	}
	else
	{
		return lcdY;

	}

}

PT_PageDesc FindAppointNode ( int PageNum )
{
	PT_PageDesc P_PageNode;

	P_PageNode = g_ptPageHead;


	while ( P_PageNode )
	{
		if ( P_PageNode->PageNum == PageNum )
		{
			//P_PointNode = P_PageNode;
			//memcpy(P_PointNode,P_PageNode,sizeof(T_PageDesc));
			return P_PageNode;
		}

		P_PageNode = P_PageNode->nextPageDesc;
	}

	return NULL;

}


int RelocateFontPos ( PT_FontBitMap ptFontBitMap )
{
	int iLcdY;
	int iDeltaX,iDeltaY;

	if ( ptFontBitMap->iYMax > gUserDisPlayMode->iYres )
	{
		return -1; //满页了
	}

	if ( ptFontBitMap->iXMax > gUserDisPlayMode->iXres )
	{
		iLcdY = LineFeed ( ptFontBitMap->iNextOriginY );
		if ( 0==iLcdY )
		{
			return -1;//满页了
		}
		else
		{

			/* 没满页 */
			iDeltaX = 0 - ptFontBitMap->iCurOriginX;
			iDeltaY = iLcdY - ptFontBitMap->iCurOriginY;

			ptFontBitMap->iCurOriginX  += iDeltaX;
			ptFontBitMap->iCurOriginY  += iDeltaY;

			ptFontBitMap->iNextOriginX += iDeltaX;
			ptFontBitMap->iNextOriginY += iDeltaY;

			ptFontBitMap->iXLeft += iDeltaX;
			ptFontBitMap->iXMax  += iDeltaX;

			ptFontBitMap->iYTop  += iDeltaY;
			ptFontBitMap->iYMax  += iDeltaY;

			return 0;


		}


	}

	return 0;

}



int OpenTextFile ( char* pcfileName )
{
	g_iFdTextFile = open ( pcfileName,O_RDONLY ); //只读方式打开文件
	if ( g_iFdTextFile < 0 )
	{
		DBG_PRINTF ( "open text file error\r\n" );
		return -1;
	}

	if ( fstat ( g_iFdTextFile, &tStat ) ) //获得统计信息
	{
		DBG_PRINTF ( "can't get fstat\n" );
		return -1;
	}

	//mmap把文件内容映射到一块内存
	g_pucTextFileMem = ( unsigned char* ) mmap ( NULL,tStat.st_size,PROT_READ, MAP_SHARED, g_iFdTextFile, 0 );
	if ( g_pucTextFileMem == ( unsigned char* )-1 )
	{
		DBG_PRINTF ( "can't mmap for text file\n" );
		return -1;
	}

	g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size; //得到文件结尾指针

	gUserEncodingOper =  SelectEncodingOprForFile ( g_pucTextFileMem ); //根据文件编码得到合适的编码方式
	if ( gUserEncodingOper )
	{
		g_pucLcdFirstPosAtFile = g_pucTextFileMem + gUserEncodingOper->iHeadLen; //得到第一个字符的位置
		return 0;
	}
	else
	{
		return -1;
	}


}

int GetDispResolution ( int* pXres,int* pYres )
{
	if ( gUserDisPlayMode )
	{
		*pXres = gUserDisPlayMode->iXres;
		*pYres = gUserDisPlayMode->iYres;
		return 0;
	}
	else
	{
		return -1;
	}



}


int SetTextAttr ( char* HzkFile,char* DisplayMode, unsigned int Size )
{
	int iError,iRet=1;
	PT_FontOpr ptFontOpr,ptTmp;


	g_dwFontSize = Size; //显示字体大小

	/********初始化显示*********/
	gUserDisPlayMode =  GetDispOpr ( DisplayMode ); //根据显示结构体的名字得到对应结构体
	if ( NULL == gUserDisPlayMode )
	{
		DBG_PRINTF ( "GetDispOpr error\r\n" );
		return -1;
	}
	iError =  gUserDisPlayMode->DeviceInit();
	if ( iError == -1 )
	{
		DBG_PRINTF ( "Display init error\r\n" );
	}
	/***********************/


	ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead; //得到此编码方式能支持的点阵-字库


	while ( ptFontOpr )
	{
		if ( strcmp ( ptFontOpr->name, "ascii" ) == 0 )
		{
			iError = ptFontOpr->FontInit ( NULL, Size );
		}
		else if ( strcmp ( ptFontOpr->name, "gbk" ) == 0 ) //当字符大小为16时会选用hzk16获得编码
		{
			iError = ptFontOpr->FontInit ( HzkFile, Size );
		}
		else
		{
			iError = ptFontOpr->FontInit ( HzkFile, Size );
		}

		DBG_PRINTF ( "%s, %d\n", ptFontOpr->name, iError );

		ptTmp = ptFontOpr->ptNext;

		if ( iError == 0 )
		{
			/* 比如对于ascii编码的文件, 可能用ascii字体也可能用gbk字体,
			 * 所以只要有一个FontInit成功, SetTextDetail最终就返回成功
			 */
			iRet = 0;
		}
		else
		{
			DelFontOprFrmEncoding ( gUserEncodingOper, ptFontOpr ); //删除初始化不成功的字库节点
		}
		ptFontOpr = ptTmp;
	}

	return iRet;

}

void PutInPageList ( PT_PageDesc P_PageNew )
{
	PT_PageDesc P_PageNode;

	if ( !g_ptPageHead )
	{
		P_PageNew->PageNum = 1; //记录页码
		g_ptPageHead = P_PageNew;

	}
	else
	{
		P_PageNode = g_ptPageHead;
		while ( P_PageNode->nextPageDesc ) //找到链表的终点
		{
			P_PageNode = P_PageNode->nextPageDesc;
		}

		P_PageNew->PageNum = P_PageNode->PageNum+1;

		P_PageNode->nextPageDesc = P_PageNew; //将新节点放入链表的末尾
		P_PageNew->prePageDesc = P_PageNode;	 //将最后第二个节点链接为最后一个节点的前一个节点
	}

	DBG_PRINTF ( "Cur Paga Num:%d\r\n",P_PageNew->PageNum );

}

int showPointPage ( int pageNum )
{

	int iError;
	//T_PageDesc T_PageNode;
	PT_PageDesc P_PageNode;

	P_PageNode = FindAppointNode ( pageNum );

	if ( P_PageNode == NULL )
	{
		return -1;
	}


	iError = ShowOnePage ( P_PageNode->CurPageStart );


	if ( iError == 0 )
	{
		g_ptCurPage = P_PageNode;

	}

	/*

	 P_PageNode = g_ptPageHead;
	while ( P_PageNode)
	{
		if ( P_PageNode->PageNum == pageNum )
		{
			break; //跳出循环
		}

		P_PageNode = P_PageNode->nextPageDesc;
	}

	iError = ShowOnePage ( P_PageNode->CurPageStart );


	if ( iError == 0 )
		{
			g_ptCurPage = P_PageNode;

		}


	*/






	return 0;

}

int ShowNextPage ( void )
{
	unsigned char* pucTextFileMemCurPos; //当前页的起始位置
	PT_PageDesc P_PageNode;
	int iError;

	if ( g_ptCurPage )
	{
		pucTextFileMemCurPos = g_ptCurPage->NextPageStart;
	}
	else
	{
		pucTextFileMemCurPos = g_pucLcdFirstPosAtFile;
	}

	iError = ShowOnePage ( pucTextFileMemCurPos ); //显示一页内容

	if ( iError == 0 ) //等于0代表当前页显示完毕
	{
		if ( ! ( g_ptCurPage&&g_ptCurPage->nextPageDesc ) )
		{

			P_PageNode = ( PT_PageDesc )  malloc ( sizeof ( T_PageDesc ) );

			if ( P_PageNode )
			{
				P_PageNode->CurPageStart = pucTextFileMemCurPos;
				P_PageNode->NextPageStart = g_pucNextPageStart;
				P_PageNode->nextPageDesc = NULL;
				P_PageNode->prePageDesc  = NULL;

				g_ptCurPage = P_PageNode;
				PutInPageList ( g_ptCurPage ); //放入页链表中
			}
			else
			{
				printf ( "malloc PageDesc error\r\n" );
				return -1;

			}

		}
		else
		{
			g_ptCurPage = g_ptCurPage->nextPageDesc;
		}


	}

	return 0;
}

int ShowPrePage ( void )
{
	int iError;

	if ( ( !g_ptCurPage ||! ( g_ptCurPage->prePageDesc ) ) )
	{
		return -1;

	}

	iError = ShowOnePage ( g_ptCurPage->prePageDesc->CurPageStart );

	if ( iError == 0 )
	{
		g_ptCurPage = g_ptCurPage->prePageDesc;


	}

	return 0;
}


//显示一页电子书内容，直到满页或者文本文件结束才会退出
int ShowOnePage ( unsigned char* Position )
{
	int bHasNotClrSceen = 1; //一页清字符标志
	int iError;
	int iLen;
	unsigned int dwCode;
	T_FontBitMap tFontBitMap;
	int bHasGetCode = 0;
	PT_FontOpr ptFontOpr;
	unsigned char* pTextStart;

	tFontBitMap.iCurOriginX = 0;
	tFontBitMap.iCurOriginY = g_dwFontSize;

	pTextStart = Position;//g_pucLcdFirstPosAtFile;

	//一次只能处理一个字节
	while ( 1 )
	{

		iLen = gUserEncodingOper->GetCodeFrmBuf ( pTextStart, g_pucTextFileMemEnd, &dwCode );//取得对应字体编码
		//DBG_PRINTF ( "dwCode : %d\r\n",dwCode );
		if ( 0 == iLen )
		{
			/* 文件结束 */
			if ( bHasGetCode ) //当显示字符之后 进入此处 判断文件結束后
			{
				g_pucNextPageStart = pTextStart;
				DBG_PRINTF ( "file end\r\n" );
				return 0;
			}
			else
			{
				DBG_PRINTF ( "GetCodeFrmBuf error\r\n" );
				return -1;
			}
		}
		bHasGetCode = 1;

		pTextStart+=iLen;

		if ( dwCode =='\r' ) //如果读取到的是回车换行符的话
		{
			continue;
		}
		else if ( dwCode =='\n' )
		{
			//换行
			tFontBitMap.iCurOriginX = 0;
			tFontBitMap.iCurOriginY = LineFeed ( tFontBitMap.iCurOriginY );

			if ( tFontBitMap.iCurOriginY == 0 )
			{
				g_pucNextPageStart = pTextStart;
				return 0;  //当前页已显示完毕
			}
			else
			{
				continue;
			}

		}
		else if ( dwCode == '\t' )
		{
			/* TAB键用一个空格代替 */
			dwCode = ' ';
		}



		ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead;

		while ( ptFontOpr )
		{

			iError = ptFontOpr->GetFontBitmap ( dwCode, &tFontBitMap ); //取得字体位图


			//判断接下来显示的字符是否能在一行内显示
			if ( RelocateFontPos ( &tFontBitMap ) !=0 )
			{
				g_pucNextPageStart = pTextStart;
				return 0;  //当前页已显示完毕
			}

			if ( iError != -1 ) //位图取得成功才执行显示
			{
				if ( bHasNotClrSceen ) //显示新一页时执行清屏
				{
					/* 首先清屏 */
					gUserDisPlayMode->CleanScreen ( COLOR_BACKGROUND );
					bHasNotClrSceen = 0;
				}

				if ( ShowOneFont ( &tFontBitMap ) ==-1 )
				{
					return -1;
				}
				tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX; //显示成功后确定下一个显示字符位置
				tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;
				//pucNextPageStart = pTextStart;


				break; //跳出循环
			}

			ptFontOpr= ptFontOpr->ptNext;

		}
	}

}


//显示函数
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
		DBG_PRINTF ( "ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp );
		return -1;
	}

	return 0;



}


