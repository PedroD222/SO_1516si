// SimpleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "usynch.h"
#include "List.h"

/////////////////////////////////////////////
//
// CCISEL 
// 2007-2011
//
// UThread    Library First    Test
//
// Jorge Martins, 2011
////////////////////////////////////////////
#define DEBUG

#define MAX_THREADS 10

#include <crtdbg.h>
#include <stdio.h>

#include "..\Include\Uthread.h"
//#include "..\Include\UthreadInternal.h"

///////////////////////////////////////////////////////////////
//															 //
// Test 1: N threads, each one printing its number M times //
//															 //
///////////////////////////////////////////////////////////////

ULONG Test1_Count;


VOID Test1_Thread (UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR) Argument;	

	for (Index = 0; Index < 10000; ++Index) {
	    putchar(Char);
		
		 
	    if ((rand() % 4) == 0) {
		    UtYield();
	    }	 
    }
	++Test1_Count;
}

VOID Test1 ()  {
	ULONG Index;

	
	Test1_Count = 0; 

	printf("\n :: Test 1 - BEGIN :: \n\n");

	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Test1_Thread, (UT_ARGUMENT) ('0' + Index));
	}   

	UtRun();

	_ASSERTE(Test1_Count == MAX_THREADS);
	printf("\n\n :: Test 1 - END :: \n");
}


//
// Ex6.
// Escreva programas para determinar o tempo de comutação de threads na biblioteca UThread. Para
// a medição de tempos, utilize a função da Windows API GetTickCount
//
// Baseado na versão original do enunciado
//
#define MAX_THREADS 10
#define MAX_CONXTEXT_CHANGES 10000
//
// Vars declaration
//
ULONG Question6_Count;
DWORD ContextChanges;
DWORD InitTickCount;
DWORD FinalTickCount;
DWORD TotalTicks;
DOUBLE AverageTickContextChange;


DWORD question;

VOID Question6_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;

	for (Index = 0; Index < MAX_CONXTEXT_CHANGES; ++Index) {
		putchar(Char);

		if ((rand() % 4) == 0) {
			++ContextChanges; // Increment Context Change Counter
			UtYield(); // Change Context
		}
	}
	++Question6_Count;
}

VOID Question6()  {
	ULONG Index;

	Question6_Count = 0;
	ContextChanges = 0;

	printf("\n :: Test 1 - BEGIN :: \n\n");
	InitTickCount = GetTickCount();

	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Question6_Thread, (UT_ARGUMENT)('0' + Index));
	}

	UtRun();

	FinalTickCount = GetTickCount();
	TotalTicks = FinalTickCount - InitTickCount;
	AverageTickContextChange = (DOUBLE)TotalTicks / (DOUBLE)ContextChanges;
	_ASSERTE(Question6_Count == MAX_THREADS);
	printf("\n\n There Where %d Context Changes in %d ticks \n", ContextChanges, TotalTicks);
	printf("\n\n Context Changes ocurred in average every %4.2f ticks \n", AverageTickContextChange);
	printf("\n\n :: Question 6 :: \n");
}

//ULONG Test1_Count;
ULONG YieldCounter;

VOID Ex2_Thread(UT_ARGUMENT Argument) {
	UCHAR Char;
	ULONG Index;
	Char = (UCHAR)Argument;

	for (Index = 0; Index < 10000; ++Index) {
		putchar(Char);


		if ((rand() % 4) == 0) {
			if (GetReadyQueueSize()>0)
				++YieldCounter;
			UtYield();
		}
	}
	//++Test1_Count;
	++YieldCounter;
}

VOID TestEx2() {
	ULONG Index;
	YieldCounter = 0;
	printf("\n :: Test Ex 2- BEGIN :: \n\n");
	for (Index = 0; Index < MAX_THREADS; ++Index) {
		UtCreate(Ex2_Thread, (UT_ARGUMENT)('0' + Index));
	}
	++YieldCounter;
	UtRun();
	printf("\nYieldCounter=%d\n", YieldCounter);
	printf("SwitchCounter=%d\n", UtGetSwitchCount());
	_ASSERTE(YieldCounter == UtGetSwitchCount());

	printf("\n\n :: Test Ex 2 - END :: \n");
}


VOID Func(UT_ARGUMENT argument){
	printf("\n\n :: %s - Begin :: \n", argument);
	UtDump();
	printf("\n\n :: --------- :: \n", argument);
	printf("\n\n :: %s - End :: \n", argument);
}

HANDLE Joiner;

VOID Wait_Func(UT_ARGUMENT argument){
	printf("\n\n :: %s - Begin :: \n", argument);
	UtJoin(Joiner);
	printf("\n\n :: %s - End :: \n", argument);
}

VOID Test_UtJoin() {
	LIST_ENTRY Queue;
	
	HANDLE Waiter1 = UtCreate(Wait_Func, (UT_ARGUMENT)"Waiter Thread 1");
	HANDLE Waiter2 = UtCreate(Wait_Func, (UT_ARGUMENT)"Waiter Thread 2");
	HANDLE Waiter3 = UtCreate(Wait_Func, (UT_ARGUMENT)"Waiter Thread 3");
	Joiner = UtCreate(Func, (UT_ARGUMENT)"Joiner Thread");
	UINT Count = 3;
	UtRun();
}



VOID Simple_Test_Func(UINT val){
	printf("Thread: %d",val);
}

VOID Test_Dump(){
	HANDLE t1 = UtCreate(Wait_Func, (UT_ARGUMENT)1);
	HANDLE t2 = UtCreate(Wait_Func, (UT_ARGUMENT)2);
	HANDLE t3 = UtCreate(Wait_Func, (UT_ARGUMENT)3);
	UtDump();
	UtRun();
}

int main () {
	UtInit();
	Test_UtJoin();
	getchar();
	TestEx2();
	getchar();
	
	Question6();
	 
	UtEnd();
	return 0;
}


