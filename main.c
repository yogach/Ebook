
#include <unistd.h>
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "draw.h"
#include "config.h"
#include "disp_manager.h"
#include "encoding_manager.h"
#include "fonts_manager.h"

#include <input_manager.h>
#include <debug_manager.h>

/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
int main (int argc,char * *argv)
{
	int iError;

	//char acFreetypeFile[128];
	char acDisplay[128];
	char acHzkFile[128];
	char acTextFile[128];
	unsigned int dwFontSize = 16;					//�ַ���С
	int bList = 0;


	//����������ָ��
	//ָ��ʹ������./Ebook -s 24 -d fb -h ./MSYH.TTF ./utf8_novel.txt
	while ((iError = getopt (argc,argv,"ls:f:h:d:")) != -1)
	{
		switch (iError)
		{
			case 'l':
				bList = 1; //����-l	   ��ӡ��֧���ֿ� ���뷽ʽ��
				break;

			case 's':
				dwFontSize = strtoul (optarg,NULL,0); //�ַ���תunsigned long �����ַ���С
				break;

			/*case 'f':
				strncpy ( acHzkFile, optarg, 128 ); //�����ֿ��ļ�
				acHzkFile[127] = '\0';
				break;*/
			case 'h':
				strncpy (acHzkFile,optarg,128); //����ֿ��ļ�
				acHzkFile[127] = '\0';
				break;

			break;
			case 'd':
				{
					strncpy (acDisplay,optarg,128); //�����ʾ��ʽ
					acDisplay[127] = '\0';
					break;
				}
				break;

			default:
				printf ("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n",argv[0]);
				printf ("Usage: %s -l\n",argv[0]);
				return - 1;

				break;
		}


	}


	if (!bList && (optind >= argc)) //optind ��ʾѡ���ֽ�֮��ĵ�ַ ����˴�����û������Ҫ������ļ�
	{
		printf ("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n",argv[0]);
		printf ("Usage: %s -l\n",argv[0]);
		return - 1;
	}


	iError = DebugInit ();							//��ʼ��debug
	InitDebugChanel();
	if (iError)
	{
		DBG_PRINTF ("DebugInit error!\n");
		return - 1;
	}
    


	iError = DisplayInit ();						//��ʼ���ײ���ʾ
	if (iError)
	{
		DBG_PRINTF ("DisplayInit error!\n");
		return - 1;
	}

	iError = FontsInit ();							//��ʼ���ֿ�
	if (iError)
	{
		DBG_PRINTF ("FontsInit error!\n");
		return - 1;
	}

	iError = EncodingInit ();						//��ʼ�����뷽ʽУ��
	if (iError)
	{
		DBG_PRINTF ("EncodingInit error!\n");
		return - 1;
	}

	iError = InputInit ();							//��ʼ�������豸
	if (iError)
	{
		DBG_PRINTF ("InputInit error!\n");
		return - 1;
	}


	if (bList)
	{
		printf ("supported display:\n");
		ShowDispOpr ();

		printf ("supported Font:\n");
		ShowFontOpr ();

		printf ("supported Encoding:\n");
		ShowEncodingOpr ();

		printf ("supported Input:\n");
		ShowInputOpr ();

		printf ("supported Debug:\n")
		ShowDebugOpr ();

		return 0;
	}


	strncpy (acTextFile,argv[optind],128);			//�õ��ı��ļ�����
	acTextFile[127] = '\0';

	iError = OpenTextFile (acTextFile);
	if (iError == -1)
	{
		DBG_PRINTF ("OpenTextFile error\n");
	}


	iError = SetTextAttr (acHzkFile,acDisplay,dwFontSize);
	if (iError)
	{
		DBG_PRINTF ("SetTextAttr error!\n");
		return - 1;
	}


	iError = AllInputDevicesInit ();				//���������豸
	if (iError)
	{
		DBG_PRINTF ("InputDevicesInit error!\n");
		return - 1;
	}

	//ShowOnePage(buff);
	if (ShowNextPage () == -1)
	{

		return - 1;
	}


	printf ("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit, 't' to jump: \r\n");


	while (1)
	{

		PT_InputEvent InputEvent;

		if (GetDeviceInput (InputEvent) == 0)
		{
			if (InputEvent->iAction == INPUT_VALUE_DOWN)
			{
				ShowNextPage ();
			}
			else if (InputEvent->iAction == INPUT_VALUE_UP)
			{
				ShowPrePage ();
			}
			else if (InputEvent->iAction == INPUT_VALUE_JUMP)
			{
				//printf ( "please input pagenum of Just shown\r\n" );
				//scanf("%d",&pageNum);
				showPointPage ((int) InputEvent->iVal);
			}
			else if (InputEvent->iAction == INPUT_VALUE_EXIT)
			{
				return 0;
			}

		}




	}


	return 0;

}


