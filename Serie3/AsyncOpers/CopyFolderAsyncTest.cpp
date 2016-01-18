#include "stdafx.h"
#include "AsyncOpers.h"


VOID CopyFolderTestCallback(PIOAsyncDev ah, LPVOID ctx) {
	if (!OperSuccess(ctx))
		printf("Error processing folder copy!\n");
	else
		printf("Folder Copied!\n");
	SetEvent((HANDLE)CtxGetUserContext(ctx));
}

VOID CopyFolderAsyncTest(LPCTSTR folderIn, LPCTSTR folderOut) {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	CopyFolderAsync(folderIn, folderOut, CopyFolderTestCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}