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

	BYTE buffer[BUFFER_SIZE];	// (circular) data buffer
	INT idxGet, idxPut;			// R/W indexes
	INT nBytes;					// Avaiable bytes;

} PIPE_SHARED, *PPIPE_SHARED;

typedef struct pipe{
	DWORD procId;
	DWORD nReaders, nWriters;
	HANDLE mtx;
	HANDLE hasData, hasSpace;
	HANDLE mapHandle;
	PPIPE_SHARED shared;
	//events of has readers and writers
} PIPE, *PPIPE;


HANDLE PipeOpenRead(TCHAR *pipeServiceName);

HANDLE PipeOpenWrite();

DWORD PipeRead(HANDLE h, PVOID pbuf, INT toRead);

DWORD PipeWrite(HANDLE h, PVOID pbuf, INT toWrite);