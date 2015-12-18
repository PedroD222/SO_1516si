#pragma once

#ifdef PIPESSERIE2_EXPORTS
#define PIPESSERIE2_API _declspec(dllexport)
#else
#define PIPESSERIE2_API _declspec(dllimport)
#endif


// pipe public definitions
#define BUFFER_SIZE		256
#define ATOMIC_RW		100		// max message size for guaranteed write atomicity


typedef struct pipe {
	INT nReaders, nWriters;		// Number of readers and writers 
								// (used among other reasons for pipe lifetime)
	
	BYTE buffer[BUFFER_SIZE];	// (circular) data buffer
	INT idxGet, idxPut;			// R/W indexes
	INT nBytes;					// Avaiable bytes;

	HANDLE hasElems, hasSpace;
	CRITICAL_SECTION cs;
	//need!!!???
	//CRITICAL_SECTION cswrite;
	// other (synchronization stuff)
	// ...
} PIPE, *PPIPE;


PIPESSERIE2_API
DWORD PipeRead(HANDLE p, PVOID pbuf, INT toRead);

PIPESSERIE2_API
DWORD PipeWrite(HANDLE p, PVOID pbuf, INT toWrite);

PIPESSERIE2_API
PPIPE PipeCreate();

PIPESSERIE2_API
HANDLE PipeOpenRead(PPIPE pipe);

PIPESSERIE2_API
HANDLE PipeOpenWrite(PPIPE pipe);

PIPESSERIE2_API
VOID PipeClose(HANDLE h);




