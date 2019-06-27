#include <unistd.h>
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "draw.h"
#include "config.h"
#include "disp_manager.h"
#include "encoding_manager.h"
#include "fonts_manager.h"

/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */

int main ( int argc, char * * argv )
{
	int iError;
	char acFreetypeFile[128];
	char acDisplay[128];
	char acHzkFile[128];
    char acTextFile[128];
	unsigned int dwFontSize = 16; //�����ַ���С
	int bList = 0;
    

	char buff[20]="���";

	//����������ָ��
	//ָ��ʹ������./show_file -s 24 -d fb -f ./MSYH.TTF  ./utf8_novel.txt
	while ( ( iError = getopt ( argc, argv, "ls:f:h:d:" ) ) != -1 )
	{
		switch ( iError )
		{

			case 'l':
				bList = 1; //����-l      ��ӡ��֧���ֿ� ���뷽ʽ��
				break;


			case 's':

				dwFontSize = strtoul ( optarg, NULL, 0 ); //�ַ���תunsigned long �����ַ���С
				break;

			case 'f':
				strncpy ( acFreetypeFile, optarg, 128 ); //�����ֿ��ļ�
				break;

			case 'h':
				strncpy ( acHzkFile, optarg, 128 ); //���HZK�ļ�
				acHzkFile[127] = '\0';
				break;
				break;

			case 'd':
			{
				strncpy ( acDisplay, optarg, 128 ); //�����ʾ��ʽ
				acDisplay[127] = '\0';
				break;
			}
			break;

			default:

				printf ( "Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0] );
				printf ( "Usage: %s -l\n", argv[0] );
				return -1;
				break;


		}


	}

	
	if (!bList && (optind >= argc))//optind ��ʾѡ���ֽ�֮��ĵ�ַ ����˴�����û������Ҫ������ļ�
	{
		printf("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
		printf("Usage: %s -l\n", argv[0]);
		return -1;
	}

	strncpy(acTextFile,argv[optind],128);//�õ��ı��ļ�����
	acTextFile[127] ='\0';

	iError = DisplayInit(); //��ʼ���ײ���ʾ
	if ( iError )
	{
		printf ( "DisplayInit error!\n" );
		return -1;
	}

	iError = FontsInit(); //��ʼ���ֿ�
	if ( iError )
	{
		printf ( "FontsInit error!\n" );
		return -1;
	}

	iError = EncodingInit();//��ʼ�����뷽ʽУ��
	if ( iError )
	{
		printf ( "EncodingInit error!\n" );
		return -1;
	}


	if ( bList )
	{
		printf ( "supported display:\n" );
		ShowDispOpr();

		printf ( "supported Font:\n" );
		ShowFontOpr();

		printf ( "supported Encoding:\n" );
		ShowEncodingOpr();

	}

    iError = OpenTextFile(acTextFile);
    if(iError == -1 )
    {
       printf ( "OpenTextFile error\n" );
	}


	iError = SetTextAttr (acHzkFile, acFreetypeFile,acDisplay,dwFontSize );
	if ( iError )
	{
		printf ( "SetTextAttr error!\n" );
		return -1;
	}

    ShowOnePage(buff);



	while ( 1 )
	{




	}


	return 0;

}
