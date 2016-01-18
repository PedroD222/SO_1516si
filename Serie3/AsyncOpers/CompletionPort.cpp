#include "stdafx.h"
#include "AsyncOpers.h"

HANDLE completionPort;

HANDLE CreateCompletionPort(int maxConcurrency) {
	return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxConcurrency);
}

BOOL CompletionPortAssociateHandle(HANDLE devHandle, ULONG_PTR completionKey) {
	HANDLE h = CreateIoCompletionPort(devHandle, completionPort, completionKey, 0);
	return h == completionPort;
}

BOOL AsyncRead(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr) {
	if (!ReadFile(sd, buffer, length, NULL, ovr) &&
		GetLastError() != ERROR_IO_PENDING) return FALSE;
	return TRUE;
}

BOOL AsyncWrite(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr) {
	if (!WriteFile(sd, buffer, length, NULL, ovr) &&
		GetLastError() != ERROR_IO_PENDING) return FALSE;
	return TRUE;
}

FORCEINLINE
static void SetOperStatus(PIOAsyncDev dev, int transferedBytes, BOOL success) {
	PIOBaseOper oper = dev->oper;

	oper->success &= success;
	oper->transferedBytes += transferedBytes;

	if (transferedBytes > 0) {
		LARGE_INTEGER pos;
		LPOVERLAPPED ovr = &dev->ovr;
		pos.HighPart = ovr->OffsetHigh; pos.LowPart = ovr->Offset;
		pos.QuadPart += transferedBytes;
		ovr->OffsetHigh = pos.HighPart; ovr->Offset = pos.LowPart;
	}
}


// função executada pelas threads associadas à IOCP
static UINT WINAPI ProcessOpers(LPVOID arg) {
	HANDLE completionPort = (HANDLE)arg;
	DWORD transferedBytes;
	PIOAsyncDev dev;
	LPOVERLAPPED ovr;
	BOOL success;

	//printf("start worker!\n");
	while (TRUE) {
		success = TRUE;
		if (!GetQueuedCompletionStatus(completionPort, &transferedBytes,
			(PULONG_PTR)&dev, &ovr, INFINITE) && GetLastError() != 38) {
			_tprintf(_T("Error %d getting activity packet!\n"), GetLastError());
			success = FALSE;
		}
		SetOperStatus(dev, transferedBytes, success);

		assert(dev->oper != NULL);
		assert(dev->oper->completeAction != NULL);
		
		dev->oper->completeAction(dev->oper, (int)transferedBytes);
		 
	}
}

// associa um conjunto de threads à IOCP
VOID CreateThreadPool(HANDLE ioPort, int nThreads) {
	for (int i = 0; i < nThreads; ++i) {
		_beginthreadex(NULL, 0, ProcessOpers, (LPVOID)ioPort, 0, NULL);
	}
}

VOID StartAsync() {
	completionPort = CreateCompletionPort(MAX_CONCURRENCY);
	CreateThreadPool(completionPort, MAX_THREADS);
}



