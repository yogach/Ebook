#include <stdio.h>
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <config.h>

static int g_iFdButton;


int KeyDevInit(void)
{  
    //打开驱动设备
    g_iFdButton = open(BUTTON_DEVICE_NAME, O_RDWR); //打开/dev/buttons驱动


}

int KeyDevExit(void)
{
    close(g_iFdButton);//关闭驱动设备

}

int KeyGetInputEvent(PT_InputEvent ptInputEvent)
{
  //设置类型
  ptInputEvent->iType =  INPUT_TYPE_KEY;

  //获取按键键值 需使用阻塞方式读取


  


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

