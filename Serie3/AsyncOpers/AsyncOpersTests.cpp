// ReadAsync.cpp : Defines the entry point for the console application.
//
// Contains functions to read line from files asynchronously

#include "stdafx.h"

#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID writeAsyncTest() {
	LARGE_INTEGER offset = {0};
	printf("TEST WRITE ASYNC\n");
	WriteAsyncTest({ "Sistemas operativos - 15/16 si" }, 30, offset);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3) {
		_tprintf(_T("usage: asyncTest <file1> <file2>"));
		return 1;
	}
	StartAsync();
	//FileCopyAsyncTest(argv[1], argv[2]);
	//FileDumpAsyncTest(argv[1]);
	writeAsyncTest();
	getchar();
	return 0;
}

