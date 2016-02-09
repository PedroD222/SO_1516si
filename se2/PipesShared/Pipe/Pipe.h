// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PIPE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PIPE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PIPE_EXPORTS
#define PIPE_API __declspec(dllexport)
#else
#define PIPE_API __declspec(dllimport)
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

	DWORD procId;
	//DWORD nReaders, nWriters;
	
} PIPE_SHARED, *PPIPE_SHARED;

typedef struct pipe{
	HANDLE mapHandle;
	PPIPE_SHARED shared;
	DWORD mode;
	HANDLE hasData, hasSpace, waitReaders, waitWriters;
	HANDLE mtx;
} PIPE, *PPIPE;

PIPE_API
HANDLE PipeOpenRead(TCHAR *pipeServiceName);

PIPE_API
HANDLE PipeOpenWrite(TCHAR *pipeServiceName);

PIPE_API
PPIPE PipeCreate(TCHAR *pipeServiceName);

PIPE_API
DWORD PipeWrite(HANDLE h, PVOID pbuf, INT toWrite);

PIPE_API
DWORD PipeRead(HANDLE h, PVOID pbuf, INT toWrite);

PIPE_API
VOID PipeClose(HANDLE h);