// ReadAsync.cpp : Defines the entry point for the console application.
//
// Contains functions to read line from files asynchronously

#include "stdafx.h"

#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID CopyFileAsyncTest(LPCTSTR fileIn, LPCTSTR fileOut) {
	LARGE_INTEGER offset = {0};
	printf("TEST WRITE ASYNC\n");
	CopyFile2AsyncTest(fileIn, fileOut);
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
	//writeAsyncTest();
	CopyFileAsyncTest(argv[1], argv[2]);
	getchar();
	return 0;
}

