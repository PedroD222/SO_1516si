#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID CopyFileCallback(PIOAsyncDev ah, LPVOID ctx) {
	if (!OperSuccess(ctx))
		printf("Error processing copy!\n");
	else {
		printf("File Copied!\n");
		printf("Bytes transferidos: %d\n", CtxGetTransferedBytes(ctx));
	}
	SetEvent((HANDLE)CtxGetUserContext(ctx));
}

VOID CopyFile2AsyncTest(LPCTSTR fileIn, LPCTSTR fileOut) {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	CopyFile2Async(fileIn, fileOut, CopyFileCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}