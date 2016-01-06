// ParallelFilter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define NPROCS 4
#define NELEMSLIST 5

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
		/*
		test aux in exclusion because head maybe null
		when head = head->next
		*/
		if (aux == NULL) {
			LeaveCriticalSection(&cs);
			
			return 0;
		}
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
	int nprocs = si.dwNumberOfProcessors;
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
	return args->elemPredicate;
}

BOOL IsOdd(VOID * item) {
	int n = (int)item;
	return n % 2;
}

VOID testParallel() {
	ListNode * head;
	
	int numbers[NELEMSLIST];
	int expected = 0;
	ListNode * aux = (ListNode*)malloc(sizeof(ListNode));
	head = aux;

	for (int i = 0; i < NELEMSLIST; i++) {
		 numbers[i] = i;
		if (IsOdd((void *)numbers[i]))
			expected++;
		aux->val = (void *)numbers[i];
		aux->next = (ListNode*)malloc(sizeof(ListNode));
	
		if (i<NELEMSLIST-1)
			aux = aux->next;
	}
	aux->next = NULL;
	int odd = ParallelFilterCount(head, IsOdd);
	printf("--------------------------------------\n");
	if (odd == expected)
		printf("TEST PASSED!!\n");
	else 
		printf("TEST FAILED!!\n");
}

int main()
{
	testParallel();
	getchar();
    return 0;
}

