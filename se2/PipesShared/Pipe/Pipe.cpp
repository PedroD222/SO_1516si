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

TCHAR szName[] = TEXT("PIPE");
#define PIPE_MUTEX_LOCK "PipeMutex"
#define PIPE_EVENT_WAITING_READERS "WaitReaders"
#define PIPE_EVENT_WAITING_WRITERS "WaitWriters"
#define PIPE_HAS_DATA_EVENT "EmptyPipeEv"
#define PIPE_HAS_SPACE_EVENT "fullPipeEv"

static VOID PipeInit(PPIPE p, TCHAR * pipeServiceName) {
	printf("PipeInit partially implemented!\n");
	//InitializeCriticalSection(&p->cs);
	int err;
	int sz = sizeof(PIPE_SHARED);
	//SeCreateG
	p->mapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(PIPE_SHARED), szName);
	err = GetLastError();
	if (p->mapHandle == NULL)
		goto error;
	


	p->shared = (PPIPE_SHARED)MapViewOfFile(p->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);// sizeof(PIPE_SHARED));
	//int **addr = &(p->shared);
	if (p->shared == NULL)
		goto error;
	err = GetLastError();
	p->shared->procId = GetCurrentProcessId();

	p->shared->nReaders = p->shared->nWriters = 0;
	p->shared->idxGet = p->shared->idxPut = p->shared->nBytes = 0;
	
	SECURITY_DESCRIPTOR desc;
	/*desc.
	SECURITY_ATTRIBUTES secur;
	secur.lpSecurityDescriptor = */

	if ((p->mtx = CreateMutex(NULL, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		goto error;
	err = GetLastError();
	if ((p->hasSpace = CreateEvent(NULL, TRUE, TRUE, _T(PIPE_HAS_SPACE_EVENT))) == NULL)
		goto error;
	err = GetLastError();
	if ((p->hasData = CreateEvent(NULL, TRUE, FALSE, _T(PIPE_HAS_DATA_EVENT))) == NULL)
		goto error;
	err = GetLastError();
	if ((p->waitReaders = CreateEvent(NULL, TRUE, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL)
		goto error;
	err = GetLastError();
	if ((p->waitWriters = CreateEvent(NULL, TRUE, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL)
		goto error;
	err = GetLastError();
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
	WaitForSingleObject(p->hasSpace, INFINITE);
	err = GetLastError();
	int largerThanAtomic = toWrite - ATOMIC_RW;

	for (;;) {
		WaitForSingleObject(p->mtx, INFINITE);
		err = GetLastError();
		if (BUFFER_SIZE - p->shared->nBytes >= toWrite) {
			//ReleaseMutex(shared->mtx);
			break;
		}

		if (BUFFER_SIZE - p->shared->nBytes >= ATOMIC_RW) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		ReleaseMutex(p->mtx);
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
		SetEvent(p->hasData);
		err = GetLastError();
	}
	if (p->shared->nBytes == BUFFER_SIZE) {
		ResetEvent(p->hasSpace);
		err = GetLastError();
	}
	ReleaseMutex(p->mtx);
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
	pipe->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
	if (pipe->mapHandle == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->shared = (PPIPE_SHARED)MapViewOfFile(pipe->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->mtx = OpenMutex(MUTEX_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		return NULL;
	//if ((pipe->mtx = CreateMutex(NULL, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		//return NULL;
	err = GetLastError();
	WaitForSingleObject(pipe->mtx, INFINITE);
	err = GetLastError();
	if ((pipe->waitReaders = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->waitWriters = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->hasSpace = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_HAS_SPACE_EVENT))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->hasData = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_HAS_DATA_EVENT))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	pipe->shared->nWriters += 1;
	SetEvent(pipe->waitWriters);
	err = GetLastError();
	
	ReleaseMutex(pipe->mtx);
	//ReleaseMutex(pipe->shared->mtx);
	err = GetLastError();
	pipe->mode = PIPE_WRITE;
	DWORD val = WaitForSingleObject(pipe->waitReaders, INFINITE);
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
	WaitForSingleObject(p->hasData, INFINITE);
	err = GetLastError();
	int can_read_atomic = toRead - ATOMIC_RW;

	for (;;) {
		WaitForSingleObject(p->mtx, INFINITE);
		err = GetLastError();
		if (p->shared->nBytes - ATOMIC_RW >= 0) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		if (p->shared->nBytes - toRead >= 0) {
			//ReleaseMutex(shared->mtx);
			break;
		}
		ReleaseMutex(p->mtx);
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
		SetEvent(p->hasSpace); err = GetLastError();
	}
		
	if (p->shared->nBytes == 0) {
		ResetEvent(p->hasData);
		err = GetLastError();
	}
	ReleaseMutex(p->mtx);
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

	pipe->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
	if (pipe->mapHandle == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->shared = (PPIPE_SHARED)MapViewOfFile(pipe->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
		return NULL;
	err = GetLastError();
	if ((pipe->mtx = OpenMutex(MUTEX_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		return NULL;
	/*if ((pipe->mtx = CreateMutex(NULL, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
	*/	//return NULL;
	//DuplicateHandle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pipe->shared->procId), pipe->shared->)
	err = GetLastError();
	WaitForSingleObject(pipe->mtx, INFINITE);
	err = GetLastError();
	if ((pipe->waitReaders = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->waitWriters = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->hasSpace = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_HAS_SPACE_EVENT))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	if ((pipe->hasData = OpenEvent(EVENT_ALL_ACCESS | SYNCHRONIZE, FALSE, _T(PIPE_HAS_DATA_EVENT))) == NULL){
		ReleaseMutex(pipe->mtx);
		return NULL;
	}
	err = GetLastError();
	pipe->shared->nReaders += 1;
	SetEvent(pipe->waitReaders);
	err = GetLastError();
	
	ReleaseMutex(pipe->mtx);
	err = GetLastError();
	pipe->mode = PIPE_READ;
	WaitForSingleObject(pipe->waitWriters, INFINITE);
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
	//PPIPE_SHARED shared = pipe->shared;

	if (pipe->hasData != NULL)
		CloseHandle(pipe->hasData);
	if (pipe->hasSpace != NULL)
		CloseHandle(pipe->hasSpace);
	if (pipe->waitReaders != NULL)
		CloseHandle(pipe->waitReaders);
	if (pipe->waitWriters != NULL)
		CloseHandle(pipe->waitWriters);

	if (UnmapViewOfFile(pipe->mapHandle) == 0)
		return;

	free(pipe->shared);
	free(pipe);
}


VOID PipeClose(HANDLE h) {
	PPIPE pipe = (PPIPE)h;
	PPIPE_SHARED shared = pipe->shared;

	WaitForSingleObject(pipe->mtx, INFINITE);
	if (pipe->mode == PIPE_READ) {
		shared->nReaders--;
	}
	else
		if (pipe->mode == PIPE_WRITE) {
			shared->nWriters--;
		}

	if (shared->nWriters == 0 && shared->nReaders == 0) {
		ReleaseMutex(pipe->mtx);
		PipeDestroy(pipe);
		return;
	}
	if (shared->nReaders == 0)
		ResetEvent(pipe->waitReaders);
	if (shared->nWriters == 0)
		ResetEvent(pipe->waitWriters);
	ReleaseMutex(pipe->mtx);
}


