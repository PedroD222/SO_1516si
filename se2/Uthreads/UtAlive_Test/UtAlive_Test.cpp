// UtAlive_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define MAX_THREADS 2

BOOL threadJoinedalive;
BOOL threadRunalive;
BOOL thReadyalive;

VOID Test1_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;
	HANDLE h = UtSelf();
	threadRunalive = UtAlive(h);

	for (Index = 0; Index < 100; ++Index) {
		putchar(Char);
		if (Index == 10) {
			UtYield();
			thReadyalive = UtAlive(h);
		}
		if (Index == 20) {
			UtJoin(UtSelf());
			threadJoinedalive = UtAlive(h);
		}
	}
}

VOID Test1() {
	ULONG Index;

	int stSize = 4 * 1024;
	printf("\n :: Test 1 - BEGIN :: \n\n");
	UCHAR c = 'a';
	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Test1_Thread, (UT_ARGUMENT)('0' + Index), (&c), stSize);
	}

	UtRun();

	_ASSERTE(threadJoinedalive);
	_ASSERTE(threadRunalive);
	_ASSERTE(thReadyalive);
	printf("\n\n :: Test 1 - END :: \n");
}


int main()
{
	UtInit();
	Test1();
	getchar();
	UtEnd();
	return 0;
}


