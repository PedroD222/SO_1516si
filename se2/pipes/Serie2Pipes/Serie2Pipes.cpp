// PipesSerie2.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "serie2pipes.h"

// Pipe handle related definitions

// pipe operation modes
#define PIPE_READ 1
#define PIPE_WRITE 2

// the HANDLE returned in PipenOpenRead and PipeOpenWrite operation
// is a pointer to PipeHandle structure
typedef struct PipeHandle {
	DWORD mode;  /* PIPE_READ or PIPE_WRITE */
	PPIPE pipe;
} PIPE_HANDLE, *PPIPE_HANDLE;


// returns a pipe handle for read/write access
static PPIPE_HANDLE PHCreate(PPIPE pipe, SHORT mode) {
	PPIPE_HANDLE phandle = (PPIPE_HANDLE)malloc(sizeof(PIPE_HANDLE));
	phandle->mode = mode;
	phandle->pipe = pipe;
	return phandle;
}

//
// Selected internal Pipe operations
//

// pipe initialization
static VOID PipeInit(PPIPE p) {
	printf("PipeInit partially implemented!\n");
	p->nReaders = p->nWriters = 0;
	p->idxGet = p->idxPut = p->nBytes = 0;
	
	p->hasSpace = CreateEvent(NULL, TRUE, TRUE, _T("fullPipeEv"));
	p->hasElems = CreateEvent(NULL, TRUE, FALSE, _T("EmptyPipeEv"));
	InitializeCriticalSection(&p->cs);
	// other (synchronization, for instance) initialization stuf....
	// .... 
}

// pipe resources release 
static VOID PipeDestroy(PPIPE p) {
	printf("PipeDestroy not implemented!\n");
	DeleteCriticalSection(&p->cs);
	free(p);
}

// pipe read internal operation
static DWORD PipeReadInternal(PPIPE p, PVOID pbuf, INT toRead) {
	printf("PipeReadInternal not implemented!\n");
	WaitForSingleObject(p->hasElems, INFINITE);
	EnterCriticalSection(&p->cs);
	if (p->nBytes == 0 && p->nWriters == 0) {
		LeaveCriticalSection(&p->cs);
		return 0;
	}
	LeaveCriticalSection(&p->cs);
	//verify here?? or before if
	//WaitForSingleObject(p->hasElems, INFINITE);

	EnterCriticalSection(&p->cs);
	int byteread =0;
	BYTE pb[BUFFER_SIZE]; BYTE aux;
	//int nb = (BUFFER_SIZE - p->idxGet);
	
	while (byteread< toRead && byteread < p->nBytes) {
		aux = p->buffer[p->idxGet];
		pb[byteread++] = p->buffer[p->idxGet];
		p->idxGet = (p->idxGet++) % BUFFER_SIZE;
		if (p->idxGet == BUFFER_SIZE)
			p->idxGet = 0;
		if (aux != p->buffer[p->idxGet] && byteread<toRead)
			printf("erro");
	}
	memcpy(pbuf, pb, byteread);
	//p->idxGet = (p->idxGet + byteread) % BUFFER_SIZE;
	p->nBytes = p->nBytes - byteread;
	/*BEFORE
	if (p->nBytes < BUFFER_SIZE)
		SetEvent(p->hasSpace);a reset a has elems*/
	if (p->nBytes < BUFFER_SIZE)
		SetEvent(p->hasSpace);
	if (p->nBytes == 0) {
		ResetEvent(p->hasElems);
	}
	
	LeaveCriticalSection(&p->cs);
	//end here
	return byteread;
}

// pipe write internal operation
static DWORD PipeWriteInternal(PPIPE p, PVOID pbuf, INT toWrite) {
	printf("PipeWriteInternal not implemented!\n");

	WaitForSingleObject(p->hasSpace, INFINITE);
	//in exclusion
	//counter c bytes lidos para sair de cs apos byteatomicwrite reached and after reenter
	//verifier possibly write on space available
	EnterCriticalSection(&p->cs);
	int bytewrite = 0;
	PBYTE pb =(PBYTE) pbuf;
	
	while (bytewrite < ATOMIC_RW && p->nBytes <= BUFFER_SIZE) {
		p->buffer[p->idxPut] = *(pb + bytewrite);
		bytewrite++;
		p->idxPut = ((p->idxPut++) % BUFFER_SIZE);
		if (p->idxPut == BUFFER_SIZE)
			p->idxPut = 0;
		p->nBytes++;
	}
		
	if (p->nBytes >= 1) {
		SetEvent(p->hasElems);
	}
	if (p->nBytes == BUFFER_SIZE) {
		ResetEvent(p->hasSpace);
	}
	LeaveCriticalSection(&p->cs);
	return bytewrite;
	
	//end exclusion to atomic write
	//enter exclusion 
	//atomics??
	/*EnterCriticalSection(&p->cs);
	while (bytewrite < toWrite && p->nBytes < BUFFER_SIZE) {
		p->buffer[p->idxPut] = pb[bytewrite++];
		p->idxPut = (p->idxPut++) % BUFFER_SIZE;
		p->nBytes++;
	}
	if (p->nBytes == 1) {
		ResetEvent(p->empty);
	}
	if (p->nBytes == BUFFER_SIZE) {
		SetEvent(p->full);
	}
	
	LeaveCriticalSection(&p->cs);*/
	//end exclusion
	//return bytewrite;
}


/*------------------------------------------------------
 * Writes in a pipe via the handle returned from PipeOpenWrite.
 * In case of success returns the number of written  (toWrite) bytes.
 * The calling thread is blocked while pipe is full, except if there are no
 * readers, returning the number of already written bytes in this case.
 * Besides pipe must have a minimum message size thta is atomically written 
 * supporting concurrently multiple writers 
 *--------------------------------------------------------------------*/
DWORD PipeWrite(HANDLE h, PVOID pbuf, INT toWrite) {
	PPIPE_HANDLE ph = (PPIPE_HANDLE)h;

	return PipeWriteInternal(ph->pipe, pbuf, toWrite);
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

/*---------------------------------------------
 * Creates and initializes a new pipe
 *----------------------------------------------*/
PPIPE PipeCreate() {
	PPIPE pres = (PPIPE)malloc(sizeof(PIPE));
	PipeInit(pres);
	return pres;
}

/*----------------------------------------------------
* Returns and HANDLE supporting  pipe access for read
*-----------------------------------------------------*/
HANDLE PipeOpenRead(PPIPE pipe) {
	// To implement: add open for read logic here
	printf("PipeOpenRead just partially implemented!\n");
	EnterCriticalSection(&pipe->cs);
	pipe->nReaders++;
	LeaveCriticalSection(&pipe->cs);
	return PHCreate(pipe, PIPE_READ);
}

/*----------------------------------------------------
 * Returns and HANDLE supporting  pipe access for write
 *-----------------------------------------------------*/
HANDLE PipeOpenWrite(PPIPE pipe) {
	// To implement: add open for read logic here 
	printf("PipeOpenWrite just partially implemented!\n");
	EnterCriticalSection(&pipe->cs);
	pipe->nWriters++;
	LeaveCriticalSection(&pipe->cs);
	return PHCreate(pipe, PIPE_WRITE);
}

/*-------------------------------------------------------------
 * Close the handle suppporting pipe access
 * the pipe is destroyed when there no readers nor writers.
 *-------------------------------------------------------------*/
VOID PipeClose(HANDLE h) {
	// To implement 
	printf("PipeClose not implemented!\n");
	
	PPIPE_HANDLE p = (PPIPE_HANDLE)h;
	EnterCriticalSection(&p->pipe->cs);
	if (p->mode == PIPE_READ){
		p->pipe->nReaders--;
		//LeaveCriticalSection(&p->pipe->cs);
	}else
		if (p->mode == PIPE_WRITE) {
			p->pipe->nWriters--;	
			//LeaveCriticalSection(&p->pipe->cs);
		}
	//EnterCriticalSection(&p->pipe->cs);
	if (p->pipe->nReaders == 0 && p->pipe->nWriters == 0) {
		LeaveCriticalSection(&p->pipe->cs);
		PipeDestroy(p->pipe);
	}
	LeaveCriticalSection(&p->pipe->cs);
}