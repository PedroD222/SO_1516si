#include "stdafx.h"
#include "AsyncOpers.h"

// state of the copy machine state
typedef enum copyAction { Read, Write } CopyAction;

// buffer size for copying
#define CP_BUF_SIZE (4096*16)

typedef struct ReadLineAsyncOper {
	IOBaseOper base;
	CopyAction action;			// copy state
	
} ReadLineAsyncOper, *PReadLineAsyncOper;

// called for successful/unsuccessfull copy termination
VOID TerminateReadLine(PReadLineAsyncOper op) {
	InvokeCallbackAndReleaseOper(&op->base);
}

/* the machine state implementation for copy
VOID ReadLineAsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	PReadLineAsyncOper aop = (PReadLineAsyncOper)op;
	if (!OperSuccess(op)) {
		TerminateReadLine(aop);
		return;
	}
}*/

VOID InitReadLineOper(PReadLineAsyncOper aop, PIOAsyncDev dev, PCallback cb, LPVOID uctx) {
	InitBase(&aop->base, dev, cb, uctx, NULL);
	
	aop->action = Read;
	dev->idRead = dev->szline = 0;
	dev->nSpaceAvailable = CP_BUF_SIZE;
}

BOOL IsEndOfLine(LPVOID ah, DWORD begin) {
	PIOAsyncDev d = (PIOAsyncDev)ah;
	BYTE b;
	
	for (DWORD i = begin; i < d->idRead; i++) {
		b = d->buffer[i];
		if (b == 10 || b == 13) {
			d->szline = i;
			return TRUE;
		}
	}
	return FALSE;
}

VOID ReadLineCb(PIOAsyncDev ah, LPVOID ctx) {
	
	PReadLineAsyncOper read = (PReadLineAsyncOper)CtxGetUserContext(ctx);
	DWORD trans = CtxGetTransferedBytes(ctx);
	ah->nSpaceAvailable = ah->nSpaceAvailable - trans;
	DWORD begin = ah->idRead;
	ah->idRead += trans;
	
	//quando nao encontra o fim de linha
	if (!IsEndOfLine(ah, begin)) {
		LARGE_INTEGER off = { 0 };
		off.LowPart = ah->idRead;
		ReadAsync(ah, off, ah->buffer+ah->idRead, 256, ReadLineCb, read);
	}
	else
		read->base.userCb(ah, &read->base);
}

VOID ReadLineAsync(PIOAsyncDev dev, PCallback cb, LPVOID ctx) {
	assert(cb != NULL);
	PReadLineAsyncOper aop = (PReadLineAsyncOper)malloc(sizeof(ReadLineAsyncOper));
	
	InitReadLineOper(aop, dev, cb, ctx);
	
	DWORD szline = 256;
	LARGE_INTEGER init = { 0 };
	
	ReadAsync(aop->base.aHandle, init, dev->buffer, szline, ReadLineCb, aop);
}