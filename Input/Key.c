#include <stdio.h>
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <config.h>

static int g_iFdButton;


int KeyDevInit(void)
{  
    //�������豸
    g_iFdButton = open(BUTTON_DEVICE_NAME, O_RDWR); //��/dev/buttons����
    if(g_iFdButton < 0 )
    {
		DBG_PRINTF("can't open %s\r\n",BUTTON_DEVICE_NAME);
    }

}

int KeyDevExit(void)
{
    close(g_iFdButton);//�ر������豸

}

int KeyGetInputEvent(PT_InputEvent ptInputEvent)
{
  unsigned char key_val = 0;
  //��������
  ptInputEvent->iType =  INPUT_TYPE_KEY;

  //��ȡ������ֵ ʹ��������ʽ��ȡ 
  read(g_iFdButton,&key_val,1);
  gettimeofday(&ptInputEvent->tTime, NULL);//��ȡ��ǰ��ȷʱ��д��tTime��

  //key����ֵ�����0x81-0x84 �����ʱ�����Ӱ���״̬�ָ����ɿ� 
  //����ֻ��У�鰴��״̬����
  switch(key_val)
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


static T_InputOpr g_tKeyOpr = {
	.name          = "stdin",
	.DeviceInit    = KeyDevInit,
	.DeviceExit    = KeyDevExit,
	.GetInputEvent = KeyGetInputEvent,
};



int KeyInit(void)
{
   return RegisterInputOpr(&g_tKeyOpr);
}

