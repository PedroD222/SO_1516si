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
	
	// other (synchronization, for instance) initialization stuf....
	// .... 
}

// pipe resources release 
static VOID PipeDestroy(PPIPE p) {
	printf("PipeDestroy not implemented!\n");
}

// pipe read internal operation
static DWORD PipeReadInternal(PPIPE p, PVOID pbuf, INT toRead) {
	printf("PipeReadInternal not implemented!\n");
	return 0;
}

// pipe write internal operation
static DWORD PipeWriteInternal(PPIPE p, PVOID pbuf, INT toWrite) {
	printf("PipeWriteInternal not implemented!\n");
	return 0;
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
	return PHCreate(pipe, PIPE_READ);
}

/*----------------------------------------------------
 * Returns and HANDLE supporting  pipe access for write
 *-----------------------------------------------------*/
HANDLE PipeOpenWrite(PPIPE pipe) {
	// To implement: add open for read logic here 
	printf("PipeOpenWrite just partially implemented!\n");
	return PHCreate(pipe, PIPE_WRITE);
}

/*-------------------------------------------------------------
 * Close the handle suppporting pipe access
 * the pipe is destroyed when there no readers nor writers.
 *-------------------------------------------------------------*/
VOID PipeClose(HANDLE h) {
	// To implement 
	printf("PipeClose not implemented!\n");
}

