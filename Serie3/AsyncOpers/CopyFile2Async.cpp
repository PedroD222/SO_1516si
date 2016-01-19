#include "stdafx.h"
#include "AsyncOpers.h"

// state of the copy machine state
typedef enum copyAction { Read, Write } CopyAction;

// buffer size for copying
#define CP_BUF_SIZE (4096*16)

typedef struct CopyFileAsyncOper {
	IOBaseOper base;
	BYTE buffer[CP_BUF_SIZE];	// transfer buffer
	CopyAction action;			// copy state
	PIOAsyncDev adOut;			// the destination file
} CopyFileAsyncOper, *PCopyFileAsyncOper;

// called for successful/unsuccessfull copy termination
VOID TerminateCopyFile(PCopyFileAsyncOper op) {
	PIOAsyncDev fileIn = op->base.aHandle;
	PIOAsyncDev fileOut = op->adOut;
	InvokeCallbackAndReleaseOper(&op->base);
	if (fileIn != NULL) CloseAsync(fileIn);
	if (fileOut != NULL) CloseAsync(fileOut);
}

// the machine state implementation for copy
VOID CopyFileAsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	PCopyFileAsyncOper aop = (PCopyFileAsyncOper)op;
	if (!OperSuccess(op)) {
		TerminateCopyFile(aop);
		return;
	}
	if (aop->action == Read) {
		if (transferedBytes == 0) { // EOF
			TerminateCopyFile(aop);
			return;
		}
		else {
			aop->action = Write;
			if (!AsyncWrite(aop->adOut->dev, aop->buffer, transferedBytes, &aop->adOut->ovr)) {
				OperSetError(op);
				TerminateCopyFile(aop);
			}
		}
	}
	else {
		aop->action = Read;
		if (!AsyncRead(aop->base.aHandle->dev, aop->buffer,CP_BUF_SIZE, &aop->base.aHandle->ovr)) {
			OperSetError(op);
			TerminateCopyFile(aop);
		}
	}
}

VOID InitCopyFileOper(PCopyFileAsyncOper aop, PCallback cb, LPVOID uctx) {
	InitBase(&aop->base, NULL, cb, uctx, CopyFileAsyncCompleteAction);
	aop->action = Read;
	aop->adOut = NULL;
}

BOOL CopyFile2Async(LPCTSTR file, // pathname do ficheiro origem
	LPCTSTR fileOut, // pathname do ficheiro destino
	PCallback cb, // função a invocar na conclusão da operação
	LPVOID ctx) { // contexto (user) a passar à função de callback
	PIOAsyncDev origin = NULL;
	PCopyFileAsyncOper copyFileAsync = (PCopyFileAsyncOper)malloc(sizeof(CopyFileAsyncOper));
	InitCopyFileOper(copyFileAsync, cb, ctx);
	origin = OpenAsync(file);
	copyFileAsync->adOut = CreateAsync(fileOut);
	if (copyFileAsync == NULL && origin == NULL && copyFileAsync->adOut == NULL) {
		TerminateCopyFile(copyFileAsync);
		return FALSE;
	}	
	/*copyFileAsync->base.aHandle = origin;
	origin->oper = &copyFileAsync->base;*/
	LARGE_INTEGER offset = { 0 };
	if (!ReadAsync(origin, offset, copyFileAsync->buffer, CP_BUF_SIZE, cb, ctx)) {
		TerminateCopyFile(copyFileAsync);
		return FALSE;
	}
	if (!WriteAsync(copyFileAsync->adOut, offset, copyFileAsync->buffer, CP_BUF_SIZE, cb, ctx)){
		TerminateCopyFile(copyFileAsync);
		return FALSE;
	}
	TerminateCopyFile(copyFileAsync);
	return TRUE;
}