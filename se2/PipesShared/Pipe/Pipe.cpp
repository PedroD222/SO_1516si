// Pipe.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Pipe.h"


#include "stdafx.h"

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



	p->shared = (PPIPE_SHARED)MapViewOfFile(p->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
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
	if ((p->shared->waitReaders = CreateEvent(NULL, TRUE, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL)
		goto error;
	if ((p->shared->waitWriters = CreateEvent(NULL, TRUE, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL)
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
	//PPIPE_SHARED shared = p->shared;
	int err;
	WaitForSingleObject(p->shared->hasSpace, INFINITE);
	err = GetLastError();
	int largerThanAtomic = toWrite - ATOMIC_RW;

	for (;;) {
		WaitForSingleObject(p->shared->mtx, INFINITE);
		err = GetLastError();
		if (BUFFER_SIZE - p->shared->nBytes >= toWrite) {
			//ReleaseMutex(shared->mtx);
			break;
		}

		if (BUFFER_SIZE - p->shared->nBytes >= ATOMIC_RW) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		ReleaseMutex(p->shared->mtx);
	}
	int byteWrite = 0;
	PBYTE pb = (PBYTE)pbuf;

	//WaitForSingleObject(shared->mtx, INFINITE);
	while (byteWrite < ATOMIC_RW && p->shared->nBytes < BUFFER_SIZE && byteWrite<toWrite) {
		p->shared->buffer[p->shared->idxPut] = *(pb + byteWrite);
		byteWrite++;
		p->shared->idxPut = (++p->shared->idxPut) % BUFFER_SIZE;
		p->shared->nBytes++;
	}

	if (p->shared->nBytes >= 1) {
		SetEvent(p->shared->hasData);
		err = GetLastError();
	}
	if (p->shared->nBytes == BUFFER_SIZE) {
		ResetEvent(p->shared->hasSpace);
		err = GetLastError();
	}
	ReleaseMutex(p->shared->mtx);
	err = GetLastError();
	if (largerThanAtomic > 0)
		return byteWrite + PipeWriteInternal(p, pb + byteWrite, largerThanAtomic);
	return byteWrite;

}

DWORD PipeWrite(HANDLE h, PVOID pbuf, INT toWrite) {
	//PPIPE_HANDLE ph = (PPIPE_HANDLE)h;

	return PipeWriteInternal(((PPIPE)h), pbuf, toWrite);
}



HANDLE PipeOpenWrite(TCHAR *pipeServiceName){
	HANDLE procHandle = NULL;
	PPIPE pipe = (PPIPE)malloc(sizeof(PIPE));
	int err;
	if (pipe == NULL)
		return NULL;
	pipe->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pipeServiceName);
	if (pipe->mapHandle == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->shared = (PPIPE_SHARED)MapViewOfFile(pipe->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->shared->mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		return NULL;
	err = GetLastError();
	WaitForSingleObject(pipe->shared->mtx, INFINITE);
	err = GetLastError();
	if ((pipe->shared->waitReaders = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->shared->waitWriters = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->shared->hasSpace = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_HAS_SPACE_EVENT))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->shared->hasData = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_HAS_DATA_EVENT))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	SetEvent(pipe->shared->waitWriters);
	err = GetLastError();
	pipe->shared->nWriters += 1;
	ReleaseMutex(pipe->shared->mtx);
	err = GetLastError();
	pipe->mode = PIPE_WRITE;
	DWORD val = WaitForSingleObject(pipe->shared->waitReaders, INFINITE);
	err = GetLastError();
	return pipe;
}

static DWORD PipeReadInternal(PPIPE p, PVOID pbuf, INT toRead){

	//PPIPE_SHARED shared = p->shared;
	printf("before mtx");
	//WaitForSingleObject(p->shared->mtx, INFINITE);
	if (p->shared->nBytes == 0 && p->shared->nWriters == 0){
		//ReleaseMutex(p->shared->mtx);
		return 0;
	}
	int err;
	WaitForSingleObject(p->shared->hasData, INFINITE);
	err = GetLastError();
	int can_read_atomic = toRead - ATOMIC_RW;

	for (;;) {
		WaitForSingleObject(p->shared->mtx, INFINITE);
		err = GetLastError();
		if (p->shared->nBytes - ATOMIC_RW >= 0) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		if (p->shared->nBytes - toRead >= 0) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		ReleaseMutex(p->shared->mtx);
	}
	printf("to read");
	int byteRead = 0;
	BYTE pb[BUFFER_SIZE];
	//WaitForSingleObject(shared->mtx, INFINITE);
	while (byteRead< toRead && byteRead < p->shared->nBytes && byteRead<ATOMIC_RW) {
		pb[byteRead++] = p->shared->buffer[p->shared->idxGet];
		p->shared->idxGet = (++p->shared->idxGet) % BUFFER_SIZE;
	}
	memcpy(pbuf, pb, byteRead);

	p->shared->nBytes = p->shared->nBytes - byteRead;

	if (p->shared->nBytes < BUFFER_SIZE){
		SetEvent(p->shared->hasSpace); err = GetLastError();
	}
		
	if (p->shared->nBytes == 0) {
		ResetEvent(p->shared->hasData);
		err = GetLastError();
	}
	ReleaseMutex(p->shared->mtx);
	err = GetLastError();
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
	//PPIPE_HANDLE ph = (PPIPE_HANDLE)h;

	return PipeReadInternal(((PPIPE)h), pbuf, toWrite);
}

HANDLE PipeOpenRead(TCHAR *pipeServiceName){
	HANDLE procHandle = NULL;
	PPIPE pipe = (PPIPE)malloc(sizeof(PIPE));
	int err;
	if (pipe == NULL)
		return NULL;

	pipe->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, pipeServiceName);
	if (pipe->mapHandle == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->shared = (PPIPE_SHARED)MapViewOfFile(pipe->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->shared->mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		return NULL;
	err = GetLastError();
	WaitForSingleObject(pipe->shared->mtx, INFINITE);
	err = GetLastError();
	if ((pipe->shared->waitReaders = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->shared->waitWriters = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->shared->hasSpace = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_HAS_SPACE_EVENT))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->shared->hasData = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_HAS_DATA_EVENT))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}
	err = GetLastError();
	SetEvent(pipe->shared->waitReaders);
	err = GetLastError();
	pipe->shared->nReaders += 1;
	ReleaseMutex(pipe->shared->mtx);
	err = GetLastError();
	pipe->mode = PIPE_READ;
	WaitForSingleObject(pipe->shared->waitWriters, INFINITE);
	err = GetLastError();
	return pipe;
}



//static PPIPE_HANDLE PHCreate(PPIPE pipe, SHORT mode) {
//	PPIPE_HANDLE phandle = (PPIPE_HANDLE)malloc(sizeof(PIPE_HANDLE));
//	phandle->mode = mode;
//	phandle->pipe = pipe;
//	return phandle;
//}


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


VOID PipeClose(HANDLE h) {
	PPIPE pipe = (PPIPE)h;
	PPIPE_SHARED shared = pipe->shared;

	WaitForSingleObject(shared->mtx, INFINITE);
	if (pipe->mode == PIPE_READ) {
		shared->nReaders--;
	}
	else
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


