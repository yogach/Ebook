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
static int g_dwFontSize; //�����С
static int g_iFdTextFile; //�ļ�ָ��
static unsigned char *g_pucTextFileMem;
static unsigned char *g_pucTextFileMemEnd;
static unsigned char *g_pucLcdFirstPosAtFile;//��һ���ַ���λ��
struct stat tStat;


int ShowOneFont ( PT_FontBitMap ptFontBitMap );

int OpenTextFile(char* pcfileName)
{
     g_iFdTextFile = open(pcfileName,O_RDONLY); //ֻ����ʽ���ļ�
     if(g_iFdTextFile < 0)
     {
         printf("open text file error\r\n");
		 return -1;
	 }

	 if(fstat(g_iFdTextFile, &tStat)) //���ͳ����Ϣ
	 {
		printf("can't get fstat\n");
		return -1;
	 }

     //mmap���ļ�����ӳ�䵽һ���ڴ�
	 g_pucTextFileMem = (unsigned char* )mmap(NULL,tStat.st_size,PROT_READ, MAP_SHARED, g_iFdTextFile, 0);
    if (g_pucTextFileMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap for text file\n");
		return -1;
	}

	 g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size; //�õ��ļ���βָ��

	 gUserEncodingOper =  SelectEncodingOprForFile(g_pucTextFileMem); //�����ļ�����õ����ʵı��뷽ʽ 
	 if (gUserEncodingOper)
	 {
		 g_pucLcdFirstPosAtFile = g_pucTextFileMem + gUserEncodingOper->iHeadLen; //�õ���һ���ַ���λ��
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

    /********��ʼ����ʾ�ṹ��*********/
	gUserDisPlayMode =  GetDispOpr ( DisplayMode ); //������ʾ�ṹ������ֵõ���Ӧ�ṹ��
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


    ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead; //�õ��˱��뷽ʽ��֧�ֵĵ���-�ֿ�

	
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
			/* �������ascii������ļ�, ������ascii����Ҳ������gbk����, 
			 * ����ֻҪ��һ��FontInit�ɹ�, SetTextDetail���վͷ��سɹ�
			 */
			iRet = 0;
		}
		else
		{
			DelFontOprFrmEncoding(gUserEncodingOper, ptFontOpr); //���ע�᲻�ɹ� ɾ���ֿ�
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

    //һ��ֻ�ܴ���һ���ֽ�
	while ( 1 )
	{
		iLen = gUserEncodingOper->GetCodeFrmBuf ( pTextStart, g_pucTextFileMemEnd, &dwCode );//ȡ�ñ���
		if ( 0 == iLen )
		{
			/* �ļ����� */
			if ( bHasGetCode )//����ʾ���ַ�֮�� ��ӡ�ļ�����
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

           iError = ptFontOpr->GetFontBitmap ( dwCode, &tFontBitMap ); //ȡ������λͼ

			if ( iError != -1 )
			{
				if ( bHasNotClrSceen )
				{
					/* �������� */
					gUserDisPlayMode->CleanScreen ( COLOR_BACKGROUND );
					bHasNotClrSceen = 0;
				}

				ShowOneFont ( &tFontBitMap );
			}
			
			tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX; //��һ����ʾ�ַ�λ��
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

				if ( ucByte & ( 1<<bit ) ) //���
				{
					gUserDisPlayMode->ShowPixel ( x, y, COLOR_FOREGROUND );
				}
				else
				{
					/* ʹ�ñ���ɫ, �����軭 */
					// g_ptDispOpr->ShowPixel(x, y, 0); /* �� */
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


