// TestUtCreate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define MAX_THREADS 2

int Test1_Count;

VOID Test1_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;
	
	for (Index = 0; Index < 100; ++Index) {
		putchar(Char);
	}
	++Test1_Count;
}

VOID Test1() {
	ULONG Index;

	Test1_Count = 0;
	int stSize = 4 * 1024;
	printf("\n :: Test 1 - BEGIN :: \n\n");
	UCHAR c = 'a';
	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Test1_Thread, (UT_ARGUMENT)('0' + Index), (&c), stSize);
	}

	UtRun();

	_ASSERTE(Test1_Count == MAX_THREADS);
	printf("\n\n :: Test 1 - END :: \n");
}


int main()
{
	UtInit();
	Test1();
	getchar();
	UtExit();
    return 0;
}

