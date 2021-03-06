#include "stdafx.h"
#include "AsyncOpers.h"
#include "AsyncOpersTests.h"

VOID ReadLineCallback(PIOAsyncDev asyncDev, LPVOID ctx) {
	if (!OperSuccess(asyncDev))
		printf("Error reading line!\n");
	else {
		printf("Line Read!\n");
		PCHAR l = CtxGetLine(asyncDev);
		printf("Linha Lida: %s\n", l);
	}
	SetEvent((HANDLE)CtxGetUserContext(ctx));
}

VOID ReadLineAsyncTest() {
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	PIOAsyncDev dev = OpenAsync(_T("TestReadline.txt"));
	dev->readline = NULL;
	ReadLineAsync(dev, ReadLineCallback, evtDone);
	WaitForSingleObject(evtDone, INFINITE);
	CloseHandle(evtDone);
}