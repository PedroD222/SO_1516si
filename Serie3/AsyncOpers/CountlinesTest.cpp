#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID CountLinesCallback(PIOAsyncDev ah, LPVOID ctx) {
	if (!OperSuccess(ctx))
		printf("Error processing copy!\n");
	else {
		printf("Lines Counted!\n");
		//printf("Bytes transferidos: %d\n", CtxGetTransferedBytes(ctx));
	}
	SetEvent((HANDLE)CtxGetUserContext(ctx));
}

VOID CountLinesAsyncTest(LPCTSTR fileIn, LPCSTR match) {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	CountLinesAsync(fileIn, match, CountLinesCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}