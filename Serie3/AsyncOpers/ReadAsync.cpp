#include "stdafx.h"
#include "AsyncOpers.h"


VOID TransferAsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	InvokeCallbackAndReleaseOper(op);
}

BOOL ReadAsync(PIOAsyncDev ah, LARGE_INTEGER offset, LPVOID buffer, DWORD length, PCallback cb, LPVOID ctx) {
	PIOBaseOper aop = (PIOBaseOper)malloc(sizeof(IOBaseOper));
	
	InitBase(aop, ah, cb, ctx, TransferAsyncCompleteAction);
	ah->ovr.Offset = offset.LowPart; ah->ovr.OffsetHigh = offset.HighPart;

	if (!ReadFile(ah->dev, buffer, length, NULL, &ah->ovr) &&
		GetLastError() != ERROR_IO_PENDING) return FALSE;
	return TRUE;
}


