
#include <config.h>
#include <input_manager.h>
#include <string.h>

static PT_InputOpr g_ptInputOprHead;//输入链表表头
static T_InputEvent g_tInputEvent;

static pthread_mutex_t g_tMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_tConVar = PTHREAD_COND_INITIALIZER;


//新增节点到InputOpr链表中
int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTmp;

	if (!g_ptInputOprHead)
	{
		g_ptInputOprHead   = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptInputOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	    = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}

	return 0;
}

//线程函数
static void* InputEventTreadFunction(void* pVoid)
{
   T_InputEvent tInputEvent;

   //定义函数指针
   int (*GetInputEvent)(PT_InputEvent ptInputEvent);

   (int (*)(PT_InputEvent))pVoid;

   GetInputEvent = (int(*)(PT_InputEvent))pVoid;//转化void*指针


   while(1)
   {
         if( 0 == GetInputEvent(&tInputEvent)) //等待输入事件
         {
			 /* 唤醒主线程, 把tInputEvent的值赋给一个全局变量 */
			 /* 访问临界资源前，先获得互斥量 */
			 pthread_mutex_lock(&g_tMutex);
			 g_tInputEvent = tInputEvent;
 
			 /*  唤醒主线程 */
			 pthread_cond_signal(&g_tConVar);
 
			 /* 释放互斥量 */
			 pthread_mutex_unlock(&g_tMutex);


		 }

   }



}


int AllInputDevicesInit(void)
{
   PT_InputOpr ptTmp = g_ptInputOprHead;
   int iError = -1;

	while (ptTmp)
	{
       if(g_ptInputOprHead->DeviceInit() == 0)
       {
       	   //创建线程 将各设备的GetInputEvent 作为形参传入
		   pthread_create(&g_ptInputOprHead->tTreadID,NULL,InputEventTreadFunction,ptTmp->GetInputEvent);
           iError = 0;
	   }
	   ptTmp = ptTmp->ptNext;
	}

   return iError;
}


//获取输入事件
int GetDeviceInput(PT_InputEvent ptInputEvent)
{

    /* 休眠 */
	pthread_mutex_lock(&g_tMutex);
	pthread_cond_wait(&g_tConVar, &g_tMutex);	

	/* 被唤醒后,返回数据 */
	*ptInputEvent = g_tInputEvent;
	pthread_mutex_unlock(&g_tMutex);

	return 0;
}

//遍历InputOpr链表
void ShowInputOpr(void)
{
	int i = 0;
	PT_InputOpr ptTmp = g_ptInputOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}



int InputInit(void)
{
    int iError;

	iError = StdinInit();

	return iError;

}


