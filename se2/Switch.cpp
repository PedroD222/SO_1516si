// Switch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define NUMBEROFTHREADS 32

typedef struct Args{
	LARGE_INTEGER ini1, ini2;
	LARGE_INTEGER end1, end2;
	HANDLE evt;
}Args, *PArgs;

HANDLE threadWaitEvt;
LARGE_INTEGER frequency;

UINT WINAPI ThreadFunc(VOID * arg){
	PArgs pa = (PArgs)arg;
	QueryPerformanceCounter(&pa->ini2);
	
	
	WaitForSingleObject(pa->evt, INFINITE);
	printf("Hello from thread");
	Sleep(1000);
	QueryPerformanceCounter(&pa->end1);
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	
	HANDLE threads[NUMBEROFTHREADS];
	Args args[NUMBEROFTHREADS];
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	DWORD nprocs = si.dwNumberOfProcessors;
	QueryPerformanceFrequency(&frequency);

	for (int i = 0; i < nprocs; ++i){
		args[i].evt = CreateEvent(NULL, TRUE, FALSE, NULL);
		QueryPerformanceCounter(&args[i].ini1);
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, &args[i], 0, NULL);
	}

	for (int i = 0; i < nprocs; ++i){
		SetEvent(args[i].evt);
		WaitForSingleObject(threads[i],INFINITE);
		//CloseHandle(threads[i]);
		QueryPerformanceCounter(&args[i].end2);
		
	}
	int count = 1;
	for (int i = 0; i < nprocs; ++i){
		printf("Thread number %d:", count);
		printf("Entry time = %d \n", ((args[i].ini2.QuadPart - args[i].ini1.QuadPart) * 1000000) / frequency.QuadPart);
		printf("Exit time = %d\n", ((args[i].end2.QuadPart - args[i].end1.QuadPart) * 1000000) / frequency.QuadPart);
		printf("-------------------- \n");
		++count;
	}

	getchar();
	return 0;
}

