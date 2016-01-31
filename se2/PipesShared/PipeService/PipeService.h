#pragma once


#ifdef PIPESSERIE2_EXPORTS
#define PIPESSERIE2_API _declspec(dllexport)
#else
#define PIPESSERIE2_API _declspec(dllimport)
#endif


// pipe public definitions
#define BUFFER_SIZE		256
#define ATOMIC_RW		100		// max message size for guaranteed write atomicity



typedef struct pipe_shared {
	INT nReaders, nWriters;		// Number of readers and writers 
	// (used among other reasons for pipe lifetime)
	HANDLE hasData, hasSpace, waitReaders, waitWriters;
	BYTE buffer[BUFFER_SIZE];	// (circular) data buffer
	INT idxGet, idxPut;			// R/W indexes
	INT nBytes;					// Avaiable bytes;

	DWORD procId;
	//DWORD nReaders, nWriters;
	HANDLE mtx;
} PIPE_SHARED, *PPIPE_SHARED;

typedef struct pipe{
	HANDLE mapHandle;
	PPIPE_SHARED shared;
	DWORD mode;
} PIPE, *PPIPE;

PIPESSERIE2_API
HANDLE PipeOpenRead(TCHAR *pipeServiceName);

PIPESSERIE2_API
HANDLE PipeOpenWrite(TCHAR *pipeServiceName);

PIPESSERIE2_API
PPIPE PipeCreate(TCHAR *pipeServiceName);

PIPESSERIE2_API
DWORD PipeRead(HANDLE h, PVOID pbuf, INT toRead);

PIPESSERIE2_API
DWORD PipeWrite(HANDLE h, PVOID pbuf, INT toWrite);

PIPESSERIE2_API
VOID PipeClose(HANDLE h);