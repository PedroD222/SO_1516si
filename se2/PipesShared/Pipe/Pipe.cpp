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
	PPIPE_SHARED shared = p->shared;

	WaitForSingleObject(shared->hasSpace, INFINITE);
	int largerThanAtomic = toWrite - ATOMIC_RW;

	if (largerThanAtomic > 0){

	}
	else{

	}


	WaitForSingleObject(shared->mtx, INFINITE);


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

	WaitForSingleObject(pipe->shared->mtx, INFINITE);
	
	if ((pipe->shared->waitReaders = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	if ((pipe->shared->waitWriters = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	SetEvent(pipe->shared->waitWriters);
	pipe->shared->nWriters += 1;
	ReleaseMutex(pipe->shared->mtx);
	pipe->mode = PIPE_WRITE;
	DWORD val = WaitForSingleObject(pipe->shared->waitReaders, INFINITE);
	DWORD err = GetLastError();
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

	if ((pipe->shared->mtx = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T(PIPE_MUTEX_LOCK))) == NULL)
		return NULL;

	WaitForSingleObject(pipe->shared->mtx, INFINITE);
	if ((pipe->shared->waitReaders = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_READERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	if ((pipe->shared->waitWriters = OpenEvent(EVENT_ALL_ACCESS, FALSE, _T(PIPE_EVENT_WAITING_WRITERS))) == NULL){
		ReleaseMutex(pipe->shared->mtx);
		return NULL;
	}

	SetEvent(pipe->shared->waitReaders);
	pipe->shared->nReaders += 1;
	ReleaseMutex(pipe->shared->mtx);
	pipe->mode = PIPE_READ;
	WaitForSingleObject(pipe->shared->waitWriters, INFINITE);
	return pipe;
}



//static PPIPE_HANDLE PHCreate(PPIPE pipe, SHORT mode) {
//	PPIPE_HANDLE phandle = (PPIPE_HANDLE)malloc(sizeof(PIPE_HANDLE));
//	phandle->mode = mode;
//	phandle->pipe = pipe;
//	return phandle;
//}




