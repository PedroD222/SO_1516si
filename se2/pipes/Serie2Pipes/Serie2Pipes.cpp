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

// pipe resources release 
static VOID PipeDestroy(PPIPE p) {
//	printf("PipeDestroy not implemented!\n");
	if (p->hasElems != NULL)
		CloseHandle(p->hasElems);
	if (p->hasSpace != NULL)
		CloseHandle(p->hasSpace);
	if (p->waitReaders != NULL)
		CloseHandle(p->waitReaders);
	if (p->waitWriters != NULL)
		CloseHandle(p->waitWriters);

	DeleteCriticalSection(&p->cs);
	free(p);
}

// pipe initialization
static VOID PipeInit(PPIPE p) {
	//printf("PipeInit partially implemented!\n");
	InitializeCriticalSection(&p->cs);

	p->nReaders = p->nWriters = p->idxGet = p->idxPut = p->nBytes = 0;
	
	if((p->hasSpace = CreateEvent(NULL, TRUE, TRUE, _T("fullPipeEv"))) == NULL)
		goto error;
	if ((p->hasElems = CreateEvent(NULL, TRUE, FALSE, _T("EmptyPipeEv"))) == NULL)
		goto error;
	if ((p->waitReaders = CreateEvent(NULL, TRUE, FALSE, _T("WaitReaders"))) == NULL)
		goto error;
	if ((p->waitWriters = CreateEvent(NULL, TRUE, FALSE, _T("WaitWriters"))) == NULL)
		goto error;
	return;
error:
	PipeDestroy(p);	
}

// pipe read internal operation
static DWORD PipeReadInternal(PPIPE p, PVOID pbuf, INT toRead) {
	//printf("PipeReadInternal not implemented!\n");

	EnterCriticalSection(&p->cs);
	if (p->nBytes == 0 && p->nWriters == 0) {
		LeaveCriticalSection(&p->cs);
		return 0;
	}
	LeaveCriticalSection(&p->cs);
	WaitForSingleObject(p->hasElems, INFINITE);
	
	int can_read_atomic = toRead - ATOMIC_RW;
	/*
	*before for(;;){...}
	EnterCriticalSection(&p->cs);
	int hasbytes_atomic = (p->nBytes - ATOMIC_RW);
	int can_read = (p->nBytes - toRead);
	LeaveCriticalSection(&p->cs);
	if(can_read_atomic > 0 )
		while (hasbytes_atomic < 0) {
			EnterCriticalSection(&p->cs);
			hasbytes_atomic = p->nBytes - ATOMIC_RW;
			LeaveCriticalSection(&p->cs);
		}	
	else 
		while (can_read < 0) {
			EnterCriticalSection(&p->cs);
			can_read = p->nBytes - toRead;
			LeaveCriticalSection(&p->cs);
		}
	*/
	for (;;) {
		EnterCriticalSection(&p->cs);	
		if (p->nBytes - ATOMIC_RW >= 0) {
			LeaveCriticalSection(&p->cs);
			break;
		}
		if (p->nBytes-toRead >= 0) {
			LeaveCriticalSection(&p->cs);
			break;
		}
		LeaveCriticalSection(&p->cs);
	}

	int byteread =0;
	BYTE pb[BUFFER_SIZE];
	EnterCriticalSection(&p->cs);
	while (byteread< toRead && byteread < p->nBytes && byteread<ATOMIC_RW) {
		pb[byteread++] = p->buffer[p->idxGet];
		p->idxGet = (++p->idxGet) % BUFFER_SIZE;
	}
	memcpy(pbuf, pb, byteread);

	p->nBytes = p->nBytes - byteread;

	if (p->nBytes < BUFFER_SIZE)
		SetEvent(p->hasSpace);
	if (p->nBytes == 0) {
		ResetEvent(p->hasElems);
	}
	LeaveCriticalSection(&p->cs);
	
	if (can_read_atomic > 0)
		return byteread + PipeReadInternal(p, pb+byteread, can_read_atomic);
	return byteread;
}

// pipe write internal operation
static DWORD PipeWriteInternal(PPIPE p, PVOID pbuf, INT toWrite) {
	//printf("PipeWriteInternal not implemented!\n");

	WaitForSingleObject(p->hasSpace, INFINITE);

	int can_write_atomic = toWrite- ATOMIC_RW;

	for (;;) {
		EnterCriticalSection(&p->cs);
		if (BUFFER_SIZE - p->nBytes >= toWrite) {
			LeaveCriticalSection(&p->cs);
			break;
		}
			
		if (BUFFER_SIZE - p->nBytes >= ATOMIC_RW) {
			LeaveCriticalSection(&p->cs);
			break;
		}
		LeaveCriticalSection(&p->cs);
	}

	/*
	* before for(;;){...}

	EnterCriticalSection(&p->cs);
	int hasSpace_atomicW = (ATOMIC_RW + p->nBytes); 
	int can_write = (p->nBytes + toWrite);
	LeaveCriticalSection(&p->cs);
	if (can_write_atomic > 0)
		while (hasSpace_atomicW > BUFFER_SIZE) {
			EnterCriticalSection(&p->cs);
			hasSpace_atomicW = (ATOMIC_RW + p->nBytes);
			LeaveCriticalSection(&p->cs);
		}
	else 
		while (can_write > BUFFER_SIZE) {
			EnterCriticalSection(&p->cs);
			can_write = (p->nBytes + toWrite);
			LeaveCriticalSection(&p->cs);
		}
		*/
	
	EnterCriticalSection(&p->cs);
	int bytewrite = 0;
	PBYTE pb =(PBYTE) pbuf;
	
	while (bytewrite < ATOMIC_RW && p->nBytes < BUFFER_SIZE && bytewrite<toWrite) {
		p->buffer[p->idxPut] = *(pb + bytewrite);
		bytewrite++;
		p->idxPut = (++p->idxPut) % BUFFER_SIZE;	
		p->nBytes++;
	}
		
	if (p->nBytes >= 1) {
		SetEvent(p->hasElems);
	}
	if (p->nBytes == BUFFER_SIZE) {
		ResetEvent(p->hasSpace);
	}
	LeaveCriticalSection(&p->cs);
	
	if (can_write_atomic > 0)
		return bytewrite + PipeWriteInternal(p, pb+bytewrite, can_write_atomic);
	return bytewrite;
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
	EnterCriticalSection(&pipe->cs);
	SetEvent(pipe->waitReaders);
	pipe->nReaders++;
	LeaveCriticalSection(&pipe->cs);
	WaitForSingleObject(pipe->waitWriters, INFINITE);
	return PHCreate(pipe, PIPE_READ);
}

/*----------------------------------------------------
 * Returns and HANDLE supporting  pipe access for write
 *-----------------------------------------------------*/
HANDLE PipeOpenWrite(PPIPE pipe) {
	EnterCriticalSection(&pipe->cs);
	SetEvent(pipe->waitWriters);	
	pipe->nWriters++;
	LeaveCriticalSection(&pipe->cs);
	WaitForSingleObject(pipe->waitReaders, INFINITE);
	return PHCreate(pipe, PIPE_WRITE);
}

/*-------------------------------------------------------------
 * Close the handle suppporting pipe access
 * the pipe is destroyed when there no readers nor writers.
 *-------------------------------------------------------------*/
VOID PipeClose(HANDLE h) {
	
	PPIPE_HANDLE p = (PPIPE_HANDLE)h;
	EnterCriticalSection(&p->pipe->cs);
	if (p->mode == PIPE_READ){
		p->pipe->nReaders--;
	}else
		if (p->mode == PIPE_WRITE) {
			p->pipe->nWriters--;	
		}
	
	if (p->pipe->nReaders == 0 && p->pipe->nWriters == 0) {
		LeaveCriticalSection(&p->pipe->cs);
		PipeDestroy(p->pipe);
		return;
	}
	if (p->pipe->nReaders ==0)
		ResetEvent(p->pipe->waitReaders);
	if (p->pipe->nWriters == 0)
		ResetEvent(p->pipe->waitWriters);
	LeaveCriticalSection(&p->pipe->cs);
}