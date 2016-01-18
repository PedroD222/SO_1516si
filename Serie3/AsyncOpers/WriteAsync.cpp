#include "stdafx.h"
#include "AsyncOpers.h"

VOID AsyncCompleteAction(PIOBaseOper op, int transferedBytes) {
	InvokeCallbackAndReleaseOper(op);
}

BOOL WriteAsync(PIOAsyncDev asyncDev, LARGE_INTEGER offset, LPVOID buffer, DWORD length, PCallback cb, LPVOID ctx) {
	PIOBaseOper baseOp = (PIOBaseOper)malloc(sizeof(IOBaseOper));

	InitBase(baseOp, asyncDev, cb, ctx, AsyncCompleteAction);
	asyncDev->ovr.Offset = offset.LowPart; asyncDev->ovr.OffsetHigh = offset.HighPart;

	if (!WriteFile(asyncDev->dev, buffer, length,NULL,&asyncDev->ovr) && GetLastError() != ERROR_IO_PENDING ) 
		return FALSE;
	return TRUE;
}