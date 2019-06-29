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
	//char acFreetypeFile[128];
	char acDisplay[128];
	char acHzkFile[128];
	char acTextFile[128];
	unsigned int dwFontSize = 16; //字符大小
	int bList = 0;


	char buff[20]="天才";

	//处理命令行指令
	//指令使用例子./Ebook -s 24 -d fb -h ./MSYH.TTF ./utf-8.txt
	while ( ( iError = getopt ( argc, argv, "ls:f:h:d:" ) ) != -1 )
	{
		switch ( iError )
		{

			case 'l':
				bList = 1; //输入-l      打印能支持字库 编码方式等
				break;


			case 's':

				dwFontSize = strtoul ( optarg, NULL, 0 ); //字符串转unsigned long 设置字符大小
				break;

			/*case 'f':
				strncpy ( acHzkFile, optarg, 128 ); //加载字库文件
				acHzkFile[127] = '\0';
				break;*/

			case 'h':
				strncpy ( acHzkFile, optarg, 128 ); //获得字库文件
				acHzkFile[127] = '\0';
				break;
				break;

			case 'd':
			{
				strncpy ( acDisplay, optarg, 128 ); //获得显示方式
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


	if ( !bList && ( optind >= argc ) ) //optind 表示选项字节之后的地址 进入此处代表没有输入要处理的文件
	{
		printf ( "Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0] );
		printf ( "Usage: %s -l\n", argv[0] );
		return -1;
	}



	iError = DisplayInit(); //初始化底层显示
	if ( iError )
	{
		DBG_PRINTF ( "DisplayInit error!\n" );
		return -1;
	}

	iError = FontsInit(); //初始化字库
	if ( iError )
	{
		DBG_PRINTF ( "FontsInit error!\n" );
		return -1;
	}

	iError = EncodingInit();//初始化编码方式校验
	if ( iError )
	{
		DBG_PRINTF ( "EncodingInit error!\n" );
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

		return 0;
	}


	strncpy ( acTextFile,argv[optind],128 ); //得到文本文件名称
	acTextFile[127] ='\0';

	iError = OpenTextFile ( acTextFile );
	if ( iError == -1 )
	{
		DBG_PRINTF ( "OpenTextFile error\n" );
	}


	iError = SetTextAttr ( acHzkFile, acDisplay,dwFontSize );
	if ( iError )
	{
		DBG_PRINTF ( "SetTextAttr error!\n" );
		return -1;
	}

	//ShowOnePage(buff);
	ShowNextPage();


	while ( 1 )
	{
	    char cOpr = 0;
		printf ( "Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: " );

		do
		{
			cOpr = getchar();
		}
		while ( ( cOpr != 'n' ) && ( cOpr != 'u' ) && ( cOpr != 'q' ) );

        if(cOpr == 'q')
           return 0;



	}


	return 0;

}
