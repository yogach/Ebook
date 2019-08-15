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


}

int KeyDevExit(void)
{
    close(g_iFdButton);//�ر������豸

}

int KeyGetInputEvent(PT_InputEvent ptInputEvent)
{
  //��������
  ptInputEvent->iType =  INPUT_TYPE_KEY;

  //��ȡ������ֵ ��ʹ��������ʽ��ȡ


  


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

