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

	if ( ( pcTSName = getenv ( "TSLIB_TSDEVICE" ) ) != NULL ) //�ӻ���������ȷ���������豸������
	{
		g_tTSDev = ts_open ( pcTSName, 0 ); /* ��������ʽ�� */
	}
	else
	{
		g_tTSDev = ts_open ( "/dev/event0", 1 ); //����ӻ����������޷���ȡ���������豸 ��������ʽ��/dev/event0
	}


	if ( !g_tTSDev )
	{
		DBG_PRINTF ( "ts_open error!\n" );
		return -1;
	}

	if ( ts_config ( g_tTSDev ) ) //����
	{
		DBG_PRINTF ( "ts_config error!\n" );
		return -1;
	}

	if ( GetDispResolution ( &giXres, &giYres ) ) //��ȡ��ʾ�豸��X Y �ֱ���
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
	struct ts_sample tSamp;         //��ͨ����ֵ
	struct ts_sample tSampPressed;  //����ʱ����Ϣ
	struct ts_sample tSampReleased; //�ɿ�ʱ����Ϣ

	/*
	*
	static struct timeval tPreTime;
	*
	*/

	while ( 1 )
	{
		iRet = ts_read ( g_tTSDev, &tSamp, 1 ); /* ��������������� */
		if ( iRet == 1 )
		{
			if ( ( tSamp.pressure > 0 ) && ( bStart == 0 ) )
			{
				//�����ǰ�ǰ���״̬ ��¼�ʼ���µĵ�
				tSampPressed = tSamp;
				bStart = 1;
			}


			if (  tSamp.pressure <= 0  ) //�����ʱ���ɿ�״̬
			{
				tSampReleased = tSamp;

				if ( bStart ) //������Ѿ����¹����ɿ�
				{
					iXgap = tSampPressed.x - tSampReleased.x ; //�õ����º󻬶��ľ���
					ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;
					ptInputEvent->tTime = tSamp.tv;


					if ( iXgap > ( giXres/5 ) ) //������º����һ��� 1/5��Ļ
					{
						ptInputEvent->iAction = INPUT_VALUE_UP;

					}
					else if ( iXgap < 0 - ( giXres/5 ) )//������º����󻬶� 1/5��Ļ
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

//��g_TouchScreen���������豸������
int TouchScreenInit ( void )
{
	return RegisterInputOpr ( &g_TouchScreen );

}
