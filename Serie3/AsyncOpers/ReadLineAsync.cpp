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
	HANDLE evtDone;
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

VOID ReadLineCallback(PIOAsyncDev ah, LPVOID ctx) {
	PReadLineAsyncOper readline = (PReadLineAsyncOper)ctx;

	DWORD transferedBytes = CtxGetTransferedBytes(ctx);
	readline->nSpaceAvailable = readline->nSpaceAvailable - transferedBytes;
	readline->idRead += transferedBytes;
	for (DWORD i = 0; i < ah->idRead; i++) {
		if (ah->buffer[i] == 10) {
			ah->idRead = i;
			ah->done = TRUE;
			break;
		}
	}
}

VOID ReadLineAsync(PIOAsyncDev dev, PCallback cb, LPVOID ctx) {
	assert(cb != NULL);
	PReadLineAsyncOper aop = (PReadLineAsyncOper)malloc(sizeof(ReadLineAsyncOper));
	InitReadLineOper(aop, cb, ctx);
	DWORD chartoread = 256;
	DWORD szline = chartoread;
	LARGE_INTEGER init = { 0 };
	
	aop->evtDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	//while (!dev->done) {
		ReadAsync(dev, init, aop->buffer, szline, ReadLineCallback, aop);
		szline += chartoread;
	//}
	
	//WaitForSingleObject(aop->evtDone, INFINITE);
		cb(dev, ctx);
	//line finish or no ???????

//	CloseHandle(aop->evtDone);
}