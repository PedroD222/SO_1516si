#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID ReadLineCallback(PIOAsyncDev asyncDev, LPVOID ctx) {
	if (!OperSuccess(ctx))
		printf("Error reading line!\n");
	else {
		printf("Line Read!\n");
		//printf("Bytes transferidos: %d\n", CtxGetTransferedBytes(ctx));
	}
	SetEvent((HANDLE)CtxGetUserContext(ctx));
}

//TODO
VOID ReadLineAsyncTest() {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	PIOAsyncDev dev = OpenAsync(_T("TestWriteAsync.txt"));
	ReadLineAsync(dev, ReadLineCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}