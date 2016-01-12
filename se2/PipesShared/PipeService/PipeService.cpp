// PipeService.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PipeService.h"


#define PIPE_READ 1
#define PIPE_WRITE 2

// the HANDLE returned in PipenOpenRead and PipeOpenWrite operation
// is a pointer to PipeHandle structure
typedef struct PipeHandle {
	DWORD mode;  /* PIPE_READ or PIPE_WRITE */
	PPIPE pipe;
	HANDLE mapHandle;
} PIPE_HANDLE, *PPIPE_HANDLE;


#define PIPE_MUTEX_LOCK "PipeMutex"
#define PIPE_EVENT_WAITING_READERS "WaitReaders"
#define PIPE_EVENT_WAITING_WRITERS "WaitWriters"
#define PIPE_HAS_DATA_EVENT "EmptyPipeEv"
#define PIPE_HAS_SPACE_EVENT "fullPipeEv"

static VOID PipeInit(PPIPE p, TCHAR * pipeServiceName) {
	printf("PipeInit partially implemented!\n");
	//InitializeCriticalSection(&p->cs);

	

	p->mapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(PIPE_SHARED), pipeServiceName);
	if (p->mapHandle == NULL)
		goto error;

	

	p->shared = (PPIPE_SHARED)MapViewOfFile(p->mapHandle, FILE_MAP_WRITE, 0, 0, 0);
	if (p->shared == NULL)
		goto error;

	p->shared->procId = GetCurrentProcessId();

	p->shared->nReaders = p->shared->nWriters = 0;
	p->shared->idxGet = p->shared->idxPut = p->shared->nBytes = 0;

	if ((p->shared->mtx = CreateMutex(NULL, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		goto error;
	if ((p->shared->hasSpace = CreateEvent(NULL, TRUE, TRUE, _T(PIPE_HAS_SPACE_EVENT))) == NULL)
		goto error;
	if ((p->shared->hasData = CreateEvent(NULL, TRUE, FALSE, _T(PIPE_HAS_DATA_EVENT))) == NULL)
		goto error;
	if ((p->shared->waitReaders = CreateEvent(NULL, TRUE, TRUE, _T(PIPE_EVENT_WAITING_READERS))) == NULL)
		goto error;
	if ((p->shared->waitWriters = CreateEvent(NULL, TRUE, TRUE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL)
		goto error;
	return;
error:
	return;
	//PipeDestroy(p);
}


/*---------------------------------------------
* Creates and initializes a new pipe
*----------------------------------------------*/
PPIPE PipeCreate(TCHAR *pipeServiceName) {
	PPIPE pres = (PPIPE)malloc(sizeof(PIPE));
	PipeInit(pres, pipeServiceName);
	return pres;
}


static DWORD PipeWriteInternal(PPIPE p, PVOID pbuf, INT toWrite){
	PPIPE_SHARED shared = p->shared;

	WaitForSingleObject(shared->hasSpace, INFINITE);
	int largerThanAtomic = toWrite - ATOMIC_RW;

	for (;;) {
		WaitForSingleObject(shared->mtx, INFINITE);
		if (BUFFER_SIZE - shared->nBytes >= toWrite) {
			//ReleaseMutex(shared->mtx);
			break;
		}

		if (BUFFER_SIZE - shared->nBytes >= ATOMIC_RW) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		ReleaseMutex(shared->mtx);
	}
	int byteWrite = 0;
	PBYTE pb = (PBYTE)pbuf;

	//WaitForSingleObject(shared->mtx, INFINITE);
	while (byteWrite < ATOMIC_RW && shared->nBytes < BUFFER_SIZE && byteWrite<toWrite) {
		shared->buffer[shared->idxPut] = *(pb + byteWrite);
		byteWrite++;
		shared->idxPut = (++shared->idxPut) % BUFFER_SIZE;
		shared->nBytes++;
	}

	if (shared->nBytes >= 1) {
		SetEvent(shared->hasData);
	}
	if (shared->nBytes == BUFFER_SIZE) {
		ResetEvent(shared->hasSpace);
	}
	ReleaseMutex(shared->mtx);

	if (largerThanAtomic > 0)
		return byteWrite + PipeWriteInternal(p, pb + byteWrite, largerThanAtomic);
	return byteWrite;

}

DWORD PipeWrite(HANDLE h, PVOID pbuf, INT toWrite) {
	PPIPE_HANDLE ph = (PPIPE_HANDLE)h;

	return PipeWriteInternal(ph->pipe, pbuf, toWrite);
}

HANDLE PipeOpenWrite(TCHAR *pipeServiceName){
	HANDLE procHandle = NULL;
	PPIPE pipe = (PPIPE)malloc(sizeof(PIPE));
	if (pipe == NULL)
		return NULL;

	pipe->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pipeServiceName);
	if (pipe->mapHandle == NULL)
		return NULL;

	if ((pipe->shared = (PPIPE_SHARED)MapViewOfFile(pipe->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		return NULL;

	if (!OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T(PIPE_MUTEX_LOCK)))
		return NULL;

	WaitForSingleObject(pipe->shared->mtx, INFINITE);
	if (!OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_READERS))) {
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	if (!OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) {
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	SetEvent(pipe->shared->waitWriters);
	pipe->shared->nWriters += 1;
	ReleaseMutex(pipe->shared->mtx);
	WaitForSingleObject(pipe->shared->waitReaders, INFINITE);
	return pipe;
}

static DWORD PipeReadInternal(PPIPE p, PVOID pbuf, INT toRead){

	PPIPE_SHARED shared = p->shared;

	WaitForSingleObject(shared->mtx, INFINITE);
	if (shared->nBytes == 0 && shared->nWriters == 0){
		ReleaseMutex(shared->mtx);
		return 0;
	}
	WaitForSingleObject(shared->hasData, INFINITE);

	int can_read_atomic = toRead - ATOMIC_RW;
	
	for (;;) {
		WaitForSingleObject(shared->mtx, INFINITE);
		if (shared->nBytes - ATOMIC_RW >= 0) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		if (shared->nBytes - toRead >= 0) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		ReleaseMutex(shared->mtx);
	}

	int byteRead = 0;
	BYTE pb[BUFFER_SIZE];
	//WaitForSingleObject(shared->mtx, INFINITE);
	while (byteRead< toRead && byteRead < shared->nBytes && byteRead<ATOMIC_RW) {
		pb[byteRead++] = shared->buffer[shared->idxGet];
		shared->idxGet = (++shared->idxGet) % BUFFER_SIZE;
	}
	memcpy(pbuf, pb, byteRead);

	shared->nBytes = shared->nBytes - byteRead;

	if (shared->nBytes < BUFFER_SIZE)
		SetEvent(shared->hasSpace);
	if (shared->nBytes == 0) {
		ResetEvent(shared->hasData);
	}
	ReleaseMutex(shared->mtx);

	if (can_read_atomic > 0)
		return byteRead + PipeReadInternal(p, pb + byteRead, can_read_atomic);
	return byteRead;
}


/*------------------------------------------------------
* Reads from a pipe via the handle returned from PipeOpenRead.
* In case of success returns the number read bytes (toRead).
* The calling thread is blocked while pipe is empty, except if there are no
* writers, returning the number of already read bytes in this case.
*--------------------------------------------------------------------*/
DWORD PipeRead(HANDLE h, PVOID pbuf, INT toWrite) {
	PPIPE_HANDLE ph = (PPIPE_HANDLE)h;

	return PipeReadInternal(ph->pipe, pbuf, toWrite);
}

HANDLE PipeOpenRead(TCHAR *pipeServiceName){
	HANDLE procHandle = NULL;
	PPIPE pipe = (PPIPE)malloc(sizeof(PIPE));
	if (pipe == NULL)
		return NULL;

	pipe->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pipeServiceName);
	if (pipe->mapHandle == NULL)
		return NULL;

	if ((pipe->shared = (PPIPE_SHARED)MapViewOfFile(pipe->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		return NULL;

	if (!OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T(PIPE_MUTEX_LOCK)))
		return NULL;

	WaitForSingleObject(pipe->shared->mtx, INFINITE);
	if (!OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_READERS))){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	if (!OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	//TODO open events hasData and Hasspace
	SetEvent(pipe->shared->waitReaders);
	pipe->shared->nReaders += 1;
	ReleaseMutex(pipe->shared->mtx);
	WaitForSingleObject(pipe->shared->waitWriters, INFINITE);
	return pipe;
}

VOID PipeClose(HANDLE h) {
	PPIPE pipe = (PPIPE)h;
	PPIPE_SHARED shared = pipe->shared;

	WaitForSingleObject(shared->mtx, INFINITE);
	if (pipe->mode == PIPE_READ) {
		shared->nReaders--;
	}else
		if (pipe->mode == PIPE_WRITE) {
			shared->nWriters--;
		}

	if (shared->nWriters == 0 && shared->nReaders == 0) {
		ReleaseMutex(shared->mtx);
		PipeDestroy(pipe);
		return;
	}
	if (shared->nReaders == 0)
		ResetEvent(shared->waitReaders);
	if (shared->nWriters == 0)
		ResetEvent(shared->waitWriters);
	ReleaseMutex(shared->mtx);
}

VOID PipeDestroy(PPIPE pipe) {
	PPIPE_SHARED shared = pipe->shared;

	if (shared->hasData != NULL)
		CloseHandle(shared->hasData);
	if (shared->hasSpace != NULL)
		CloseHandle(shared->hasSpace);
	if (shared->waitReaders != NULL)
		CloseHandle(shared->waitReaders);
	if (shared->waitWriters != NULL)
		CloseHandle(shared->waitWriters);

	if (UnmapViewOfFile(pipe->mapHandle) == 0)
		return;

	free(pipe->shared);
	free(pipe);
}
 
//static PPIPE_HANDLE PHCreate(PPIPE pipe, SHORT mode) {
//	PPIPE_HANDLE phandle = (PPIPE_HANDLE)malloc(sizeof(PIPE_HANDLE));
//	phandle->mode = mode;
//	phandle->pipe = pipe;
//	return phandle;
//}