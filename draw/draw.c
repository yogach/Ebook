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
static int g_dwFontSize; //�����С
static int g_iFdTextFile; //�ļ�ָ��
static unsigned char* g_pucTextFileMem;
static unsigned char* g_pucTextFileMemEnd;
static unsigned char* g_pucLcdFirstPosAtFile;//��һ���ַ���λ��
static struct stat tStat;
static unsigned char FirstDis;

//����
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

int RelocateFontPos ( PT_FontBitMap ptFontBitMap )
{
	int iLcdY;
    int iDeltaX,iDeltaY;

	if ( ptFontBitMap->iYMax > gUserDisPlayMode->iYres )
	{
		return -1; //��ҳ��
	}

	if ( ptFontBitMap->iXMax > gUserDisPlayMode->iXres )
	{
		iLcdY = LineFeed ( ptFontBitMap->iNextOriginY );
		if ( 0==iLcdY )
		{
			return -1;//��ҳ��
		}
		else
		{

			/* û��ҳ */
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
	g_iFdTextFile = open ( pcfileName,O_RDONLY ); //ֻ����ʽ���ļ�
	if ( g_iFdTextFile < 0 )
	{
		DBG_PRINTF ( "open text file error\r\n" );
		return -1;
	}

	if ( fstat ( g_iFdTextFile, &tStat ) ) //���ͳ����Ϣ
	{
		DBG_PRINTF ( "can't get fstat\n" );
		return -1;
	}

	//mmap���ļ�����ӳ�䵽һ���ڴ�
	g_pucTextFileMem = ( unsigned char* ) mmap ( NULL,tStat.st_size,PROT_READ, MAP_SHARED, g_iFdTextFile, 0 );
	if ( g_pucTextFileMem == ( unsigned char* )-1 )
	{
		DBG_PRINTF ( "can't mmap for text file\n" );
		return -1;
	}

	g_pucTextFileMemEnd = g_pucTextFileMem + tStat.st_size; //�õ��ļ���βָ��

	gUserEncodingOper =  SelectEncodingOprForFile ( g_pucTextFileMem ); //�����ļ�����õ����ʵı��뷽ʽ
	if ( gUserEncodingOper )
	{
		g_pucLcdFirstPosAtFile = g_pucTextFileMem + gUserEncodingOper->iHeadLen; //�õ���һ���ַ���λ��
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


	g_dwFontSize = Size; //��ʾ�����С

	/********��ʼ����ʾ*********/
	gUserDisPlayMode =  GetDispOpr ( DisplayMode ); //������ʾ�ṹ������ֵõ���Ӧ�ṹ��
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


	ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead; //�õ��˱��뷽ʽ��֧�ֵĵ���-�ֿ�


	while ( ptFontOpr )
	{
		if ( strcmp ( ptFontOpr->name, "ascii" ) == 0 )
		{
			iError = ptFontOpr->FontInit ( NULL, Size );
		}
		else if ( strcmp ( ptFontOpr->name, "gbk" ) == 0 ) //���ַ���СΪ16ʱ��ѡ��hzk16��ñ���
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
			/* �������ascii������ļ�, ������ascii����Ҳ������gbk����,
			 * ����ֻҪ��һ��FontInit�ɹ�, SetTextDetail���վͷ��سɹ�
			 */
			iRet = 0;
		}
		else
		{
			DelFontOprFrmEncoding ( gUserEncodingOper, ptFontOpr ); //ɾ����ʼ�����ɹ����ֿ�ڵ�
		}
		ptFontOpr = ptTmp;
	}

	return iRet;

}

int ShowNextPage(void)
{
   unsigned char *pucTextFileMemCurPos; //��ǰҳ����ʼλ��

   if()
   {
     
   }
   else
   {

   }

   pucTextFileMemCurPos = g_pucLcdFirstPosAtFile;



   ShowOnePage(pucTextFileMemCurPos);

}


//��ʾһҳ���������ݣ�ֱ����ҳ�����ı��ļ������Ż��˳�
int ShowOnePage ( unsigned char* Position )
{
	int bHasNotClrSceen = 1; //һҳ���ַ���־
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

	//һ��ֻ�ܴ���һ���ֽ�
	while ( 1 )
	{

		iLen = gUserEncodingOper->GetCodeFrmBuf ( pTextStart, g_pucTextFileMemEnd, &dwCode );//ȡ�ñ���
		DBG_PRINTF ( "dwCode : %d\r\n",dwCode );
		if ( 0 == iLen )
		{
			/* �ļ����� */
			if ( bHasGetCode ) //����ʾ�ַ�֮�� ����˴� �ж��ļ��Y����
			{
				DBG_PRINTF ( "file end\r\n" );
				return -1;
			}
			else
			{
				DBG_PRINTF ( "GetCodeFrmBuf error\r\n" );
				return 0;
			}
		}
		bHasGetCode = 1;

		pTextStart+=iLen; 

		if ( dwCode =='\r' ) //�����ȡ�����ǻس����з��Ļ�
		{
			continue;
		}
		else if ( dwCode =='\n' )
		{
			//����
			tFontBitMap.iCurOriginX = 0;
			tFontBitMap.iCurOriginY = LineFeed ( tFontBitMap.iCurOriginY );

			if ( tFontBitMap.iCurOriginY == 0 )
			{
				return 0;  //��ǰҳ����ʾ���
			}
			else
			{
				continue;
			}

		}
		else if ( dwCode == '\t' )
		{
			/* TAB����һ���ո���� */
			dwCode = ' ';
		}



		ptFontOpr = gUserEncodingOper->ptFontOprSupportedHead;

		while ( ptFontOpr )
		{

			iError = ptFontOpr->GetFontBitmap ( dwCode, &tFontBitMap ); //ȡ������λͼ


			//�жϽ�������ʾ���ַ��Ƿ�����һ������ʾ
			if ( RelocateFontPos ( &tFontBitMap )!=0 )
			{
                return 0;  //��ǰҳ����ʾ���
			}

			if ( iError != -1 ) //λͼȡ�óɹ���ִ����ʾ
			{
				if ( bHasNotClrSceen ) //��ʾ��һҳʱִ������
				{
					/* �������� */
					gUserDisPlayMode->CleanScreen ( COLOR_BACKGROUND );
					bHasNotClrSceen = 0;
				}

				if(ShowOneFont ( &tFontBitMap )==-1)
				{
                  return -1;
				}
				tFontBitMap.iCurOriginX = tFontBitMap.iNextOriginX; //��ʾ�ɹ���ȷ����һ����ʾ�ַ�λ��
				tFontBitMap.iCurOriginY = tFontBitMap.iNextOriginY;

				break; //����ѭ��
			}			

			ptFontOpr= ptFontOpr->ptNext;

		}
	}

}


//��ʾ����
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
		DBG_PRINTF ( "ShowOneFont error, can't support %d bpp\n", ptFontBitMap->iBpp );
		return -1;
	}

  return 0;



}


