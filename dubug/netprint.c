#include <config.h>
#include <debug_manager.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#define SERVER_PORT 5678
#define PRINT_BUF_SIZE   (16*1024)

typedef struct PrintBuff
{
	unsigned char * Buff;
    unsigned int ReadPos;
	unsigned int WritePos;
	
}T_PrintBuff;


static T_PrintBuff g_tPrintBuff;

static int g_iSocketServer;
static struct sockaddr g_tSocketServerAddr;


static pthread_t g_tSendTreadID;
static pthread_t g_tRecvTreadID;


static int isFull(void)
{
  if(g_tPrintBuff.WritePos )


}

static int isEmpty(void)
{
  if(g_tPrintBuff.ReadPos == g_tPrintBuff.WritePos)
   return 1;
  else
   return 0;

}


static int PutData(char cVal)
{
	
}

static int GetData(char *pcVal)
{
	
}


static void *NetDbgSendTreadFunction(void *pVoid)
{

  while(1)
  	{}

}

static void *NetDbgRecvTreadFunction(void *pVoid)
{
	while(1)
  	{}
}


static int NetDbgInit(void)
{
    int iRet;

	g_iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);//设置为UDP传输

    if(g_iSocketServer == -1 )
    {
      printf("socket init error\r\n");
	  return -1;

	}

	g_tSocketServerAddr.sin_family		= AF_INET;
	g_tSocketServerAddr.sin_port		= htons(SERVER_PORT);  /* host to net, short */
	g_tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset(g_tSocketServerAddr.sin_zero, 0, 8);

	//绑定socket与端口
	iRet = bind(g_iSocketServer, (const struct sockaddr *)&g_tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		printf("bind error!\n");
		return -1;
	}

    g_tPrintBuff.Buff = malloc(PRINT_BUF_SIZE);
	if(g_tPrintBuff.Buff == NULL)
	{
		printf("malloc printfbuff error!\n");
		close(g_iSocketServer);
		return -1;
	}
	
	//创建发送及接收线程
	pthread_create(&g_tSendTreadID,NULL,NetDbgSendTreadFunction,NULL);
	pthread_create(&g_tRecvTreadID,NULL,NetDbgRecvTreadFunction,NULL);
    
}

static int NetDbgExit(void)
{
  /* 关闭socket,... */
  close(g_iSocketServer);
  free(g_tPrintBuff.Buff);

}

static int NetDbgPrint(char *strData)
{
	
	
}


static T_DebugOpr g_tNetDbgOpr = {
	.name       = "netprint",
	.isCanUse   = 1,
	.DebugInit  = NetDbgInit,
	.DebugExit  = NetDbgExit,
	.DebugPrint = NetDbgPrint,
};

int NetPrintInit(void)
{
	return RegisterDebugOpr(&g_tNetDbgOpr);
}

