// Parallel.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define MAXTHREADS 32

typedef BOOL(*Filter)(VOID *item);

typedef struct ListNode {
	VOID *val; /* valor passado ao filtro */
	struct ListNode *next;
} ListNode;


BOOL IsOdd(VOID * item){
	return FALSE;
}

typedef struct Args{
	Filter filter;
	VOID *val;
	BOOL res;
} Args;

DWORD WINAPI Caller(LPVOID arg)//BOOL res, Filter filter, VOID * val)
{
	Args *args = (Args *)arg;
	args->res = args->filter(args->val);
	return 0;
}




int ParallelFilterCount(ListNode *head, Filter filter)
{
	if (head == NULL)
		return 0;
	SYSTEM_INFO sinfo;
	HANDLE threads[MAXTHREADS];
	Args args[MAXTHREADS];
	
	int i = 0;
	int sum = 0;
	ListNode * current = head;
	do{
		if (i == MAXTHREADS-1)
		{
			for (int j = 0; j < MAXTHREADS; ++j)
			{
				WaitForSingleObject(threads[i], INFINITE);
				CloseHandle(threads[i]);
				if (args[i].res == TRUE) ++sum;
			}
			i = 0;
		}
		args[i].filter = filter;
		args[i].val = current->val;
		CreateThread(NULL, 0, Caller, args + i, 0, NULL);
		current = current->next;
		++i;
	} while (current != NULL);
	
	for (int j = 0; j < MAXTHREADS; ++j)
	{
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
		if (args[i].res == TRUE) ++sum;
	}

}


int _tmain(int argc, _TCHAR* argv[])
{
	



	return 0;
}

