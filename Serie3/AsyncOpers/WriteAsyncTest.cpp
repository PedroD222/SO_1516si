#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID WriteCallback(PIOAsyncDev asyncDev, LPVOID ctx) {
	if (!OperSuccess(ctx))
		printf("Error processing copy!\n");
	else {
		printf("Buffer Copied!\n");
		printf("Bytes transferidos: %d\n", CtxGetTransferedBytes(ctx));
	}
	SetEvent((HANDLE)CtxGetUserContext(ctx));
}

VOID WriteAsyncTest(LPVOID buffer, DWORD length, LARGE_INTEGER offset) {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	PIOAsyncDev dev = OpenAsync(_T("TestWriteAsync.txt"));
	WriteAsync(dev, offset,buffer, length, WriteCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}