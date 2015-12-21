// Switch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define NUMBEROFTHREADS 32

typedef struct Args{
	LARGE_INTEGER * ini;
	LARGE_INTEGER * end;
	HANDLE evt;
}Args;

HANDLE threadWaitEvt;

DWORD WINAPI ThreadFunc(VOID * arg){
	Sleep(500);
}


int _tmain(int argc, _TCHAR* argv[])
{
	LARGE_INTEGER frequency;
	HANDLE threads[NUMBEROFTHREADS];
	Args args[NUMBEROFTHREADS];
	QueryPerformanceFrequency(&frequency);

	for (int i = 0; i < NUMBEROFTHREADS; ++i){
		args[i].evt = CreateEvent()
	}

	for (int i = 0; i < NUMBEROFTHREADS; ++i){
		SetEvent(args[i].evt);
		WaitForSingleObject(threads[i],INFINITE);
	}

	return 0;
}

