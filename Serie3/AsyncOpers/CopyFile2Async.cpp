#include "stdafx.h"
#include "AsyncOpers.h"

// state of the copy machine state
typedef enum copyAction { Read, Write } CopyAction;

// buffer size for copying
#define CP_BUF_SIZE (4096*16)

typedef struct FileCopyAsyncOper {
	IOBaseOper base;
	BYTE buffer[CP_BUF_SIZE];	// transfer buffer
	CopyAction action;			// copy state
	PIOAsyncDev adOut;			// the destination file
} FileCopyAsyncOper, *PFileCopyAsyncOper;

// called for successful/unsuccessfull copy termination
VOID TerminateFileCopy(PFileCopyAsyncOper op) {
	PIOAsyncDev fileIn = op->base.aHandle;
	PIOAsyncDev fileOut = op->adOut;
	InvokeCallbackAndReleaseOper(&op->base);
	if (fileIn != NULL) CloseAsync(fileIn);
	if (fileOut != NULL) CloseAsync(fileOut);
	free(op);
}

// the machine state implementation for copy
VOID FileCopyAsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	PFileCopyAsyncOper aop = (PFileCopyAsyncOper)op;
	if (!OperSuccess(op)) {
		TerminateFileCopy(aop);
		return;
	}
	if (aop->action == Read) {
		if (transferedBytes == 0) { // EOF
			TerminateFileCopy(aop);
			return;
		}
		else {
			aop->action = Write;
			if (!AsyncWrite(aop->adOut->dev, aop->buffer, transferedBytes, &aop->adOut->ovr)) {
				OperSetError(op);
				TerminateFileCopy(aop);
			}
		}
	}
	else {
		aop->action = Read;
		if (!AsyncRead(aop->base.aHandle->dev, aop->buffer,CP_BUF_SIZE, &aop->base.aHandle->ovr)) {
			OperSetError(op);
			TerminateFileCopy(aop);
		}
	}
}

VOID InitFileCopyOper(PFileCopyAsyncOper aop, PCallback cb, LPVOID uctx) {
	InitBase(&aop->base, NULL, cb, uctx, FileCopyAsyncCompleteAction);
	aop->action = Read;
	aop->adOut = NULL;
}

BOOL CopyFile2Async(LPCTSTR file, // pathname do ficheiro origem
	LPCTSTR fileOut, // pathname do ficheiro destino
	PCallback cb, // função a invocar na conclusão da operação
	LPVOID ctx) { // contexto (user) a passar à função de callback
	PIOAsyncDev origin;
	PFileCopyAsyncOper copyFileAsync = (PFileCopyAsyncOper)malloc(sizeof(FileCopyAsyncOper));
	InitFileCopyOper(copyFileAsync, cb, ctx);
	if (copyFileAsync == NULL && 
		(origin = OpenAsync(file)) == NULL &&
		(copyFileAsync->adOut = CreateAsync(fileOut)) == NULL) {
		TerminateFileCopy(copyFileAsync);
		return FALSE;
	}	
	LARGE_INTEGER offset = { 0 };
	if (!ReadAsync(origin, offset, copyFileAsync->buffer, CP_BUF_SIZE, cb, ctx)) {
		TerminateFileCopy(copyFileAsync);
		return FALSE;
	}
	if (!WriteAsync(copyFileAsync->adOut, offset, copyFileAsync->buffer, CP_BUF_SIZE, cb, ctx)){
		TerminateFileCopy(copyFileAsync);
		return FALSE;
	}
	TerminateFileCopy(copyFileAsync);
	return TRUE;
}