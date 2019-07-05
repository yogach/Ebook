
#include <input_manager.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>


static int StdinDevInit(void)
{
    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn off canonical mode
    tTTYState.c_lflag &= ~ICANON;
    //minimum of number input read.
    tTTYState.c_cc[VMIN] = 1;   /* 有一个数据时就立刻返回 */ 

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;
}

static int StdinDevExit(void)
{

    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn on canonical mode
    tTTYState.c_lflag |= ICANON;
	
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);	
	return 0;
}

static int StdinGetInputEvent(PT_InputEvent ptInputEvent)
{

 	char c;
	
	/* 处理数据 */
	ptInputEvent->iType = INPUT_TYPE_STDIN;
	
	c = fgetc(stdin);  /* 会休眠直到有输入 阻塞方式读取一直等待直到有返回 */
	gettimeofday(&ptInputEvent->tTime, NULL);//获取当前精确时间写入tTime中
	
	if (c == 'u')
	{
		ptInputEvent->iAction = INPUT_VALUE_UP;
	}
	else if (c == 'n')
	{
		ptInputEvent->iAction = INPUT_VALUE_DOWN;
	}
	else if (c == 'q')
	{
		ptInputEvent->iAction = INPUT_VALUE_EXIT;
	}
	else if(c == 't')
	{
        ptInputEvent->iAction = INPUT_VALUE_JUMP;  
		printf ( "please input pagenum of Just shown\r\n" );
		scanf("%d",&ptInputEvent->iAction);
	}
	else
	{
		ptInputEvent->iAction = INPUT_VALUE_UNKNOWN;
	}		
	return 0;
 }

static T_InputOpr g_tStdinOpr = {
	.name          = "stdin",
	.DeviceInit    = StdinDevInit,
	.DeviceExit    = StdinDevExit,
	.GetInputEvent = StdinGetInputEvent,
};


int StdinInit(void)
{
	return RegisterInputOpr(&g_tStdinOpr);
}

