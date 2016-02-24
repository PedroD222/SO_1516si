#include "stdafx.h"
#include "AsyncOpers.h"

typedef struct COUNTLINESOP {
	IOBaseOper base;
	DWORD nLines;
	LPCSTR match;
}countLinesOp, * PCountLinesOp;

VOID TerminateCountLines(PCountLinesOp op) {
	CloseAsync(op->base.aHandle);
	free(op);
}

VOID CountLinesCb(PIOAsyncDev ah, LPVOID ctx) {
	PCHAR line = CtxGetLine(ah);
	PCountLinesOp clines = (PCountLinesOp)CtxGetUserContext(ctx);
	if (!OperSuccess(&clines->base)) {
		OperSetError(&clines->base);
		TerminateCountLines(clines);
		return;
	}
	if (strstr(line, clines->match) != NULL)
		clines->nLines++;
	ReadLineAsync(ah, CountLinesCb, clines);
}

VOID InitCountLinesOper(PCountLinesOp aop, PIOAsyncDev dev, PCallback cb, LPVOID uctx, LPCSTR m) {
	InitBase(&aop->base, dev, cb, uctx, NULL);
	dev->readline = NULL;
	aop->match = m;
}

BOOL CountLinesAsync(LPCTSTR fileIn, // pathname do ficheiro
					LPCSTR match, // string a procurar
					PCallback cb, // fun��o a invocar na conclus�o da opera��o
					LPVOID ctx) { // contexto (user) a passar � fun��o de callback
	PCountLinesOp clines = (PCountLinesOp)malloc(sizeof(countLinesOp));
	
	PIOAsyncDev dev = OpenAsync(fileIn);
	if (dev == NULL)
		return FALSE;
	InitCountLinesOper(clines, dev, cb, ctx, match);
	ReadLineAsync(dev, CountLinesCb, clines);
	return TRUE;
}