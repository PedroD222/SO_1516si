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




HANDLE Joiner;

VOID Func(UT_ARGUMENT argument){
	printf("Joiner Thread... \n");
	int st = UtThreadState(Joiner);
	printf("Joiner Thread state: %d", st);
}

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



int main () {
	UtInit();
	Test_UtJoin();
	getchar();

	UtEnd();
	return 0;
}


