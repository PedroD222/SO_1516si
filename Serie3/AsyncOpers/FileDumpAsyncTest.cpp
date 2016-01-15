#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID FileDumpAsyncCallback(PIOAsyncDev ah, LPVOID ctx) {
	printf("File Dumped!\n");
	HANDLE evtDone = (HANDLE)ctx;
	SetEvent(evtDone);
}

VOID FileDumpAsyncTest(LPCTSTR fileIn) {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);

	FileDumpAsync(fileIn, FileDumpAsyncCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
}
