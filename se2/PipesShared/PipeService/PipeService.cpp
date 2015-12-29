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


static VOID PipeInit(PPIPE p, TCHAR * pipeServiceName) {
	printf("PipeInit partially implemented!\n");
	//InitializeCriticalSection(&p->cs);

	

	p->mapHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(PIPE_SHARED), pipeServiceName);
	if (p->mapHandle == NULL)
		goto error;

	p->procId = GetCurrentProcessId();

	p->shared = (PPIPE_SHARED)MapViewOfFile(p->mapHandle, FILE_MAP_WRITE, 0, 0, 0);
	if (p->shared == NULL)
		goto error;

	p->nReaders = p->nWriters = 0;
	p->shared->idxGet = p->shared->idxPut = p->shared->nBytes = 0;

	if ((p->mtx = CreateMutex(NULL, FALSE, _T("PipeMutex"))) == NULL)
		goto error;
	if ((p->hasSpace = CreateEvent(NULL, TRUE, TRUE, _T("fullPipeEv"))) == NULL)
		goto error;
	if ((p->hasElems = CreateEvent(NULL, TRUE, FALSE, _T("EmptyPipeEv"))) == NULL)
		goto error;
	if ((p->waitReaders = CreateEvent(NULL, TRUE, TRUE, _T("WaitReaders"))) == NULL)
		goto error;
	if ((p->waitWriters = CreateEvent(NULL, TRUE, TRUE, _T("WaitWriters"))) == NULL)
		goto error;
	return;
error:
	PipeDestroy(p);
}


/*---------------------------------------------
* Creates and initializes a new pipe
*----------------------------------------------*/
PPIPE PipeCreate(TCHAR *pipeServiceName) {
	PPIPE pres = (PPIPE)malloc(sizeof(PIPE));
	PipeInit(pres);
	return pres;
}


static DWORD PipeWriteInternal(PPIPE p, PVOID pbuf, INT toWrite){
	PPIPE_SHARED shared = p->shared;

	WaitForSingleObject(p->hasSpace, INFINITE);
	int largerThanAtomic = toWrite - ATOMIC_RW;

	if (largerThanAtomic > 0){

	}
	else{

	}


	WaitForSingleObject(p->mtx,INFINITE);


}


HANDLE PipeOpenWrite(){}

static DWORD PipeReadInternal(PPIPE p, PVOID pbuf, INT toRead){

	PPIPE_SHARED shared = p->shared;

	WaitForSingleObject(p->mtx,INFINITE);
	if (shared->nBytes == 0 && shared->nWriters == 0){
		ReleaseMutex(p->mtx);
		return 0;
	}
	WaitForSingleObject(p->hasData, INFINITE);

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

	if ((pipe->))

	if (!DuplicateHandle())

}



//static PPIPE_HANDLE PHCreate(PPIPE pipe, SHORT mode) {
//	PPIPE_HANDLE phandle = (PPIPE_HANDLE)malloc(sizeof(PIPE_HANDLE));
//	phandle->mode = mode;
//	phandle->pipe = pipe;
//	return phandle;
//}