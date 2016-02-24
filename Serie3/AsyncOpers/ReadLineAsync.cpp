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
	free(op->base.aHandle->readline);
	free(op);
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
	if (dev->readline == NULL) {
		dev->readline = (PIOReadLine)malloc(sizeof(IOReadLine));
		if (dev->readline != NULL) {
			dev->readline->idRead = dev->readline->szline = 0;
			dev->readline->nSpaceAvailable = CP_BUF_SIZE;
		}
	}
}

BOOL IsEndOfLine(LPVOID ah, DWORD begin) {
	PIOAsyncDev d = (PIOAsyncDev)ah;
	PIOReadLine readline = d->readline;
	BYTE b;
	
	for (DWORD i = begin; i < readline->idRead; i++) {
		b = readline->buffer[i];
		if (b == 10 || b == 13) {
			readline->szline = i;
			return TRUE;
		}
	}
	return FALSE;
}

VOID ReadLineCb(PIOAsyncDev ah, LPVOID ctx) {
	PReadLineAsyncOper read = (PReadLineAsyncOper)CtxGetUserContext(ctx);
	if (!OperSuccess(ctx)) {
		OperSetError(&read->base);
		TerminateReadLine(read);
		return;
	}
	DWORD trans = CtxGetTransferedBytes(ctx);
	PIOReadLine readline = ah->readline;
	readline->nSpaceAvailable = readline->nSpaceAvailable - trans;
	DWORD begin = readline->idRead;
	readline->idRead += trans;
	
	//quando nao encontra o fim de linha
	if (!IsEndOfLine(ah, begin)) {
		LARGE_INTEGER off = { 0 };
		off.LowPart = readline->idRead;
		ReadAsync(ah, off, readline->buffer+readline->idRead, 256, ReadLineCb, read);
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
	
	init.LowPart = dev->readline->idRead;
	ReadAsync(aop->base.aHandle, init, dev->readline->buffer, szline, ReadLineCb, aop);
}