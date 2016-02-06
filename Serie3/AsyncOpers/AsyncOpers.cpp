#include "stdafx.h"

#include "AsyncOpers.h"

//
// Auxiliary functions
//

void InvokeCallbackAndReleaseOper(PIOBaseOper op) {
	if (op->aHandle != NULL) op->aHandle->oper = NULL;
	op->userCb(op->aHandle, op);
	free(op);
}

// Context access

LPVOID CtxGetUserContext(LPVOID ctx) {
	PIOBaseOper aop = (PIOBaseOper)ctx;
	return aop->uCtx;
}

DWORD CtxGetTransferedBytes(LPVOID ctx) {
	PIOBaseOper aop = (PIOBaseOper)ctx;
	return aop->transferedBytes;
}

PCHAR CtxGetLine(LPVOID ctx) {
	PIOBaseOper op = (PIOBaseOper)ctx;
	PCHAR line = (PCHAR)malloc(op->aHandle->idRead*sizeof(char));
	for (DWORD i = 0; i < op->aHandle->idRead; i++) {
		line[i] = op->aHandle->buffer[i];
	}
	return line;
}

// Device access creators

// Create an AsyncDev from a file
static PIOAsyncDev OpenOrCreateAsync(LPCTSTR fileName, DWORD creationDisposition) {
	HANDLE h = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, creationDisposition, FILE_FLAG_OVERLAPPED, NULL);
	if (h == INVALID_HANDLE_VALUE) return NULL;
	PIOAsyncDev ah = (PIOAsyncDev)malloc(sizeof(IOAsyncDev));
	if (ah == NULL) { CloseHandle(h); return NULL; }
	ah->dev = h;
	ah->oper = NULL; // on creation no operation is associated with device
	// initialize corresponding overlapped structure
	ZeroMemory(&ah->ovr, sizeof(OVERLAPPED));
	// associate to completion port setting the PIOAsyncDev as the key 
	CompletionPortAssociateHandle(h, (ULONG_PTR)ah);

	return ah;
}

// Create an AsyncDev from an opened file
PIOAsyncDev OpenAsync(LPCTSTR fileName) {
	return OpenOrCreateAsync(fileName, OPEN_EXISTING);
}

// Create an AsyncDev from a created file
PIOAsyncDev CreateAsync(LPCTSTR fileName) {
	return OpenOrCreateAsync(fileName, CREATE_ALWAYS);
}

// called when the device access is no more necessary
VOID CloseAsync(PIOAsyncDev ah) {
	CloseHandle(ah->dev);
	free(ah);
}

 


