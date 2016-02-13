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

VOID InitCopyFileOper(PCopyFileAsyncOper aop, PCallback cb, LPVOID uctx) {
	//nao precisa completeAction nao tem device directamente
	InitBase(&aop->base, NULL, cb, uctx, NULL);
	aop->action = Read;
	aop->adOut = NULL;
}

VOID CFCallback(PIOAsyncDev ah, LPVOID ctx) {
	PCopyFileAsyncOper toWrite = (PCopyFileAsyncOper)ctx;
	
	DWORD transferedBytes = CtxGetTransferedBytes(ctx);
	PCopyFileAsyncOper copyFile = (PCopyFileAsyncOper)toWrite->base.uCtx;
	PCallback cb = (PCallback)copyFile->base.userCb;
	//copyFile->base.uCtx event to sinalize an write on file
	if (!WriteAsync(copyFile->adOut, {0}, copyFile->buffer, transferedBytes, cb, copyFile->base.uCtx)) {
		TerminateCopyFile(toWrite);
	}
}

BOOL CopyFile2Async(LPCTSTR file, // pathname do ficheiro origem
					LPCTSTR fileOut, // pathname do ficheiro destino
					PCallback cb, // fun��o a invocar na conclus�o da opera��o
					LPVOID ctx) { // contexto (user) a passar � fun��o de callback
	PIOAsyncDev origin = NULL;
	PCopyFileAsyncOper copyFileAsync = (PCopyFileAsyncOper)malloc(sizeof(CopyFileAsyncOper));
	InitCopyFileOper(copyFileAsync, cb, ctx);
	origin = OpenAsync(file);
	copyFileAsync->adOut = CreateAsync(fileOut);
	if (copyFileAsync == NULL && origin == NULL && copyFileAsync->adOut == NULL) {
		TerminateCopyFile(copyFileAsync);
		return FALSE;
	}	
	copyFileAsync->base.aHandle = origin;
	origin->oper = &copyFileAsync->base;
	LARGE_INTEGER offset = { 0 };
	if (!ReadAsync(copyFileAsync->base.aHandle, offset, copyFileAsync->buffer, CP_BUF_SIZE, CFCallback, copyFileAsync)) {
		TerminateCopyFile(copyFileAsync);
		return FALSE;
	}
	return TRUE;
}