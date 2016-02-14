//  Async File Copy
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
VOID TerminateFileCopy(PFileCopyAsyncOper op)  {
	PIOAsyncDev adIn = op->base.aHandle;
	PIOAsyncDev adOut = op->adOut;
	InvokeCallbackAndReleaseOper(&op->base);
	if (adIn != NULL) CloseAsync(adIn);
	if (adOut != NULL) CloseAsync(adOut);
}

// the machine state implementation for copy
VOID FileCopyAsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	PFileCopyAsyncOper aop = (PFileCopyAsyncOper)op;
	if (!OperSuccess(op))  {
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
	}
}


VOID InitFileCopyOper(PFileCopyAsyncOper aop, PCallback cb, LPVOID uctx) {
	InitBase(&aop->base, NULL, cb, uctx, FileCopyAsyncCompleteAction);
	aop->action = Read;
	aop->adOut = NULL;
}

VOID FileCopyAsync(LPCTSTR fileIn, LPCTSTR fileOut, PCallback cb, LPVOID ctx) {
	assert(cb != NULL);
	PFileCopyAsyncOper aop = (PFileCopyAsyncOper)malloc(sizeof(FileCopyAsyncOper));
	InitFileCopyOper(aop, cb, ctx);
	
	PIOAsyncDev adIn = OpenAsync(fileIn);
	if (adIn == NULL) {
		OperSetError(&aop->base);
		TerminateFileCopy(aop);
		return;
	}
	aop->base.aHandle = adIn;
	adIn->oper = &aop->base;
	PIOAsyncDev adOut = CreateAsync(fileOut);
	if (adOut == NULL) {
		OperSetError(&aop->base);
		TerminateFileCopy(aop);
		return;
	} 
	aop->adOut = adOut;
	aop->adOut->oper = &aop->base;
	if (!AsyncRead(adIn->dev, aop->buffer, BUFFER_SIZE, &adIn->ovr)) {
		OperSetError(&aop->base);
		TerminateFileCopy(aop);
	}
	 
}