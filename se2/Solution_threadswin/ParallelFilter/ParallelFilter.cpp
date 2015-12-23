// ParallelFilter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define NPROCS 4

typedef BOOL(*Filter)(VOID *item);
typedef struct ListNode {
	VOID *val; /* valor passado ao filtro */
	struct ListNode *next;
} ListNode;

typedef struct Args {
	long elemPredicate;
	Filter filter;
	ListNode * head;
} Args, * PArgs;

PArgs  args;

HANDLE threads[NPROCS];

CRITICAL_SECTION cs;

UINT WINAPI PartialFilter(LPVOID arg) {
	PArgs args = (PArgs )arg;
	ListNode * aux;
	for (;;) {
		EnterCriticalSection(&cs);
		aux = args->head;
		LeaveCriticalSection(&cs);
		if (aux == NULL)
			return 0;
		// exclusao mutua
		EnterCriticalSection(&cs);
		args->head = args->head->next;
		LeaveCriticalSection(&cs);
		if (args->filter(aux->val))
			InterlockedIncrement( &args->elemPredicate);
	}

	return 0;
}


int ParallelFilterCount(ListNode *head, Filter filter) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD nprocs = si.dwNumberOfProcessors;
	InitializeCriticalSection(&cs);
	DWORD total = 0;
	ListNode * l = head;
	args = (PArgs)malloc(sizeof(Args));
	args->head = l;
	args->filter = filter;
	args->elemPredicate = 0;
	for (int i = 0; i < nprocs; ++i) {
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, PartialFilter, args  , 0, NULL);
	}

	for (int i = 0; i < nprocs; ++i) {
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
	}
	DeleteCriticalSection(&cs);
	return args->elemPredicate;;
}

BOOL IsOdd(VOID * item) {
	return TRUE;
}

int main()
{
	ListNode * head = (ListNode*)malloc(sizeof(ListNode));
	head->val = (void *)1;
	ListNode * aux = (ListNode*)malloc(sizeof(ListNode));
	aux->val = (void *)3;
	aux->next = NULL;
	head->next = aux;
	ParallelFilterCount(head, IsOdd);
	getchar();
    return 0;
}

