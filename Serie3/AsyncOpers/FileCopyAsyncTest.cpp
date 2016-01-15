#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID FileCopyCallback(PIOAsyncDev ah, LPVOID ctx) {
	if (!OperSuccess(ctx))
		printf("Error processing copy!\n");
	else {
		printf("File Copied!\n");
		printf("Bytes transferidos: %d\n", CtxGetTransferedBytes(ctx));
	}
	SetEvent((HANDLE) CtxGetUserContext(ctx));
}

VOID FileCopyAsyncTest(LPCTSTR fileIn, LPCTSTR fileOut) {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	FileCopyAsync(fileIn, fileOut, FileCopyCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}
	 