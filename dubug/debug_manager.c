
#include <config.h>
#include <debug_manager.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static PT_DebugOpr g_ptDebugOprHead;
static int g_iDbgLevelLimit = 8;

//将DebugOpr放入链表中
int RegisterDebugOpr (PT_DebugOpr ptDebugOpr)
{
	PT_DebugOpr ptTmp;

	if (!g_ptDebugOprHead)
	{
		g_ptDebugOprHead = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}
	else 
	{
		ptTmp = g_ptDebugOprHead;

		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}

		ptTmp->ptNext = ptDebugOpr;
		ptDebugOpr->ptNext = NULL;
	}

	return 0;
}


//打印DebugOpr链表所有节点
void ShowDebugOpr (void)
{
	int i = 0;
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp)
	{
		printf ("%02d %s\n",i++,ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}


//取出指定DebugOpr节点
PT_DebugOpr GetDebugOpr (char * pcName)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp)
	{
		if (strcmp (ptTmp->name,pcName) == 0)
		{
			return ptTmp;
		}

		ptTmp = ptTmp->ptNext;
	}

	return NULL;
}


int SetDbgLevel (char * strBuf)
{


}


int SetDbgChanel (char * strBuf)
{


}


int DebugPrint (const char * pcFormat,...)
{
	va_list tArg;
	char strTmpBuf[1000];
	PT_DebugOpr ptTmp;

	
	va_start (tArg,pcFormat);
	iNum = vsprintf (strTmpBuf,pcFormat,tArg); //获取可变参数中的值
	va_end (tArg);
	strTmpBuf[iNum] = '\0';


	ptTmp = g_ptDebugOprHead;
    //执行debug链表中的打印语句
	while (ptTmp)
	{
		if ((ptTmp->DebugPrint)&&(ptTmp->isCanUse))
			ptTmp->DebugPrint (strTmpBuf);

		ptTmp = ptTmp->ptNext;

	}

}


int DebugInit (void)
{
	int iError;

	iError = StdoutInit ();
	iError |= NetPrintInit ();
	return iError;
}


//设备初始化
int InitDebugChanel (void)
{
	PT_DebugOpr ptTmp = g_ptDebugOprHead;

	while (ptTmp)
	{
		if (ptTmp->isCanUse && ptTmp->DebugInit)
		{
			ptTmp->DebugInit ();
		}

		ptTmp = ptTmp->ptNext;
	}

	return 0;
}


