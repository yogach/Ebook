#include <stdio.h>
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <config.h>


static int g_iFdButton;


static int KeyDevInit ( void )
{
	//打开驱动设备
	g_iFdButton = open ( BUTTON_DEVICE_NAME, O_RDWR ); //打开/dev/buttons驱动
	if ( g_iFdButton < 0 )
	{
		DBG_PRINTF ( "can't open %s\r\n",BUTTON_DEVICE_NAME );
		return -1;
	}

	return 0;

}

static int KeyDevExit ( void )
{
	close ( g_iFdButton ); //关闭驱动设备
	return 0;
}

static int KeyGetInputEvent ( PT_InputEvent ptInputEvent )
{
	unsigned char key_val = 0;
	//设置类型
	ptInputEvent->iType =  INPUT_TYPE_KEY;

	//DBG_PRINTF("KeyGetInputEvent\r\n");

	//获取按键键值 使用阻塞方式读取
	read ( g_iFdButton,&key_val,1 );
	gettimeofday ( &ptInputEvent->tTime, NULL ); //获取当前精确时间写入tTime中

	//DBG_PRINTF("GetKeyValue\r\n");


	//key返回值如果是0x81-0x84 代表此时按键从按下状态恢复到松开
	//我们只需校验按下状态即可
	switch ( key_val )
	{
		case 0x01:
			ptInputEvent->iAction =INPUT_VALUE_UP;

			break;
		case 0x02:
			ptInputEvent->iAction =INPUT_VALUE_EXIT;

			break;
		case 0x03:
			ptInputEvent->iAction =INPUT_VALUE_DOWN;

			break;
		default :
			ptInputEvent->iAction =INPUT_VALUE_UNKNOWN;

			break;


	}

	return 0;
}


static T_InputOpr g_tKeyOpr =
{
	.name          = "key",
	.DeviceInit    = KeyDevInit,
	.DeviceExit    = KeyDevExit,
	.GetInputEvent = KeyGetInputEvent,
};



int KeyInit ( void )
{
	return RegisterInputOpr ( &g_tKeyOpr );
}

