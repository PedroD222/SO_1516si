// UtJoin_TEST.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define MAX_THREADS 2

BOOL threadJoinedalive;

VOID Test1_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;


	for (Index = 0; Index < 10000; ++Index) {
		putchar(Char);
		if ((rand() % 4) == 0) {
			UtYield();
		}
	}
}

VOID Test2_Thread(UT_ARGUMENT Argument) {
	HANDLE h = (HANDLE)Argument;
	ULONG Index;

	for (Index = 0; Index < 10; ++Index) {
		
		if ((rand() % 4) == 0) {
			if (UtJoin(h))
				printf("Thread Joined.\n");
		}
	}
	threadJoinedalive = UtAlive(h);
}

VOID Test1() {
	ULONG Index;

	int stSize = 4 * 1024;
	printf("\n :: Test 1 - BEGIN :: \n\n");
	UCHAR c = 'a';
	//for (Index = 0; Index < MAX_THREADS; ++Index) {
	HANDLE th = UtCreate(Test1_Thread, (UT_ARGUMENT)('0'), (&c), stSize);
	//}
	c++;
	UtCreate(Test2_Thread, (UT_ARGUMENT)(th), (&c), stSize);

	UtRun();

	_ASSERTE(!threadJoinedalive);

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

