#include "stdafx.h"
#include "AsyncOpers.h"

// state of the copy machine state
typedef enum copyAction { Read, Write } CopyAction;

// buffer size for copying
#define CP_BUF_SIZE (4096*16)

typedef struct ReadLineAsyncOper {
	IOBaseOper base;
	CopyAction action;			// copy state
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
	
}

VOID InitReadLineOper(PReadLineAsyncOper aop, PIOAsyncDev dev, PCallback cb, LPVOID uctx) {
	InitBase(&aop->base, dev, cb, uctx, ReadLineAsyncCompleteAction);
	aop->action = Read;
	dev->idRead = dev->nSpaceAvailable = 0;
}

VOID ReadLineCb(PIOAsyncDev ah, LPVOID ctx) {
	PReadLineAsyncOper readline = (PReadLineAsyncOper)ctx;
	DWORD trans = CtxGetTransferedBytes(ctx);
	ah->nSpaceAvailable = ah->nSpaceAvailable - CtxGetTransferedBytes(ctx);
	//TODO getline code after not here?
	PCHAR l = CtxGetLine(ah);
	BYTE b;
	for (DWORD i = ah->idRead; i < CP_BUF_SIZE; i++) {
		b = ah->buffer[i];
		if (b == 10 || b == 13) {
			ah->idRead = i;
			ah->done = TRUE;
			break;
		}
	}
	//quando nao encontra o fim de linha
	//
	if (!ah->done) {
		LARGE_INTEGER off = { 0 };
		ah->idRead += trans;
		off.LowPart = trans;
		ReadAsync(ah, off, ah->buffer+ah->idRead, 256, ReadLineCb, NULL);
	}
		
}

VOID ReadLineAsync(PIOAsyncDev dev, PCallback cb, LPVOID ctx) {
	assert(cb != NULL);
	PReadLineAsyncOper aop = (PReadLineAsyncOper)malloc(sizeof(ReadLineAsyncOper));
	
	InitReadLineOper(aop, dev, cb, ctx);
	
	DWORD szline = 256;
	LARGE_INTEGER init = { 0 };
	
	ReadAsync(dev, init, dev->buffer, szline, ReadLineCb, aop);
}