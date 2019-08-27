#include <config.h>
#include <input_manager.h>
#include <stdlib.h>

#include <tslib.h>
#include <draw.h>

static struct tsdev* g_tTSDev;
static int giXres,giYres;

static int TouchScreenDevInit ( void )
{
	char* pcTSName;

	if ( ( pcTSName = getenv ( "TSLIB_TSDEVICE" ) ) != NULL ) //从环境变量里确定触摸屏设备的名字
	{
		g_tTSDev = ts_open ( pcTSName, 0 ); /* 以阻塞方式打开 */
	}
	else
	{
		g_tTSDev = ts_open ( "/dev/event0", 1 ); //如果从环境变量中无法获取到触摸屏设备 以阻塞方式打开/dev/event0
	}


	if ( !g_tTSDev )
	{
		DBG_PRINTF ( "ts_open error!\n" );
		return -1;
	}

	if ( ts_config ( g_tTSDev ) ) //配置
	{
		DBG_PRINTF ( "ts_config error!\n" );
		return -1;
	}

	if ( GetDispResolution ( &giXres, &giYres ) ) //获取显示设备的X Y 分辨率
	{
		return -1;
	}

	return 0;

}

static int TouchScreenDevExit ( void )
{

	return 0;
}

static int TouchScreenGetInputEvent ( PT_InputEvent ptInputEvent )
{
	int iRet,bStart=0,iXgap;
	struct ts_sample tSamp;         //普通采样值
	struct ts_sample tSampPressed;  //按下时的信息
	struct ts_sample tSampReleased; //松开时的信息

	/*
	*
	static struct timeval tPreTime;
	*
	*/

	while ( 1 )
	{
		iRet = ts_read ( g_tTSDev, &tSamp, 1 ); /* 如果无数据则休眠 */
		if ( iRet == 1 )
		{
			if ( ( tSamp.pressure > 0 ) && ( bStart == 0 ) )
			{
				//如果当前是按下状态 记录最开始按下的点
				tSampPressed = tSamp;
				bStart = 1;
			}


			if (  tSamp.pressure <= 0  ) //如果此时是松开状态
			{
				tSampReleased = tSamp;

				if ( bStart ) //如果是已经按下过又松开
				{
					iXgap = tSampPressed.x - tSampReleased.x ; //得到按下后滑动的距离
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					ptInputEvent->tTime = tSamp.tv;


					if ( iXgap > ( giXres/5 ) ) //如果按下后往右滑动 1/5屏幕
					{
						ptInputEvent->iAction = INPUT_VALUE_UP;

					}
					else if ( iXgap < 0 - ( giXres/5 ) )//如果按下后往左滑动 1/5屏幕
					{

						ptInputEvent->iAction = INPUT_VALUE_DOWN;

					}
					else
					{
                        if(iXgap < 20)
                        {
                           if(tSampPressed.x <= (giXres/3) )
						   	 ptInputEvent->iAction = INPUT_VALUE_UP;
						   else if(tSampPressed.x >= (giXres*2/3) )
						   	 ptInputEvent->iAction = INPUT_VALUE_DOWN;
						   else
						   	ptInputEvent->iAction = INPUT_VALUE_UNKNOWN;

						}
						else
						  ptInputEvent->iAction = INPUT_VALUE_UNKNOWN;

					}

					return 0;
				}
				else
			    {

				  return -1;
			    }
			}
			


		}
		else
		{
			return -1;
		}


	}
	return 0;
}


static T_InputOpr g_TouchScreen =
{
	.name = "touchscreen",
	.DeviceInit = TouchScreenDevInit,
	.DeviceExit = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,

};

//将g_TouchScreen放入输入设备链表中
int TouchScreenInit ( void )
{
	return RegisterInputOpr ( &g_TouchScreen );

}
