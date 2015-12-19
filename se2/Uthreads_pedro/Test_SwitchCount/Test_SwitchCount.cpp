// Test_SwitchCount.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#define DEBUG
#define MAX_THREADS 10

ULONG Test2_Count;

VOID Test2_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;

	//for (Index = 0; Index < 10000; ++Index) {
		putchar(Char);

		if ((rand() % 4) == 0) {
			UtYield();
			++Test2_Count;
		}
	//}
}

VOID Test2() {
	ULONG Index;

	Test2_Count = 2;

	printf("\n :: Test 2 - BEGIN :: \n\n");
	
	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Test2_Thread, (UT_ARGUMENT)('0' + Index));
	}

	UtRun();

	//_ASSERTE(Test2_Count == UtGetSwitchCount());
	printf("\n\n :: Test 2 - END :: \n");
	printf("%d - %d", UtGetSwitchCount(), Test2_Count);
}


int main()
{
	UtInit();

	Test2();
	getchar();

	UtEnd();
	return 0;
}

