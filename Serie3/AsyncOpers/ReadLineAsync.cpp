#include "stdafx.h"
#include "AsyncOpers.h"

// state of the copy machine state
typedef enum copyAction { Read, Write } CopyAction;

// buffer size for copying
#define CP_BUF_SIZE (4096*16)

typedef struct ReadLineAsyncOper {
	IOBaseOper base;
	BYTE buffer[CP_BUF_SIZE];	// transfer buffer
	CopyAction action;			// copy state
	DWORD idRead, nSpaceAvailable;
} ReadLineAsyncOper, *PReadLineAsyncOper;

// called for successful/unsuccessfull copy termination
VOID TerminateReadLine(PReadLineAsyncOper op) {
	
	InvokeCallbackAndReleaseOper(&op->base);
	
}

// the machine state implementation for copy
VOID ReadLineAsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	PReadLineAsyncOper aop = (PReadLineAsyncOper)op;
	if (!OperSuccess(op)) {
		TerminateReadLine(aop);
		return;
	}
	/*if (aop->action == Read) {
		if (transferedBytes == 0) { // EOF
			TerminateFileCopy(aop);
			return;
		}
		else {
			aop->action = Write;
			if (!AsyncWrite(aop->adOut->dev, aop->buffer,
				transferedBytes, &aop->adOut->ovr)) {
				OperSetError(op);
				TerminateFileCopy(aop);
			}
		}
	}
	else {
		aop->action = Read;
		if (!AsyncRead(aop->base.aHandle->dev, aop->buffer,
			CP_BUF_SIZE, &aop->base.aHandle->ovr)) {
			OperSetError(op);
			TerminateFileCopy(aop);
		}
	}*/
}


VOID InitReadLineOper(PReadLineAsyncOper aop, PCallback cb, LPVOID uctx) {
	InitBase(&aop->base, NULL, cb, uctx, ReadLineAsyncCompleteAction);
	aop->action = Read;
	aop->idRead = 0;
	aop->nSpaceAvailable = CP_BUF_SIZE;
}

VOID ReadLineAsync(PIOAsyncDev dev, PCallback cb, LPVOID ctx) {
	assert(cb != NULL);
	PReadLineAsyncOper aop = (PReadLineAsyncOper)malloc(sizeof(ReadLineAsyncOper));
	InitReadLineOper(aop, cb, ctx);
	DWORD szline = 256;
	LARGE_INTEGER init = { 0 };
	
	HANDLE evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	ReadAsync(dev, init, aop->buffer, szline, cb, ctx);
	
	WaitForSingleObject(evtDone, INFINITE);

	//line finish or no ???????

	CloseHandle(evtDone);
}