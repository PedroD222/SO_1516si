#pragma once

// é usado o nível de concorrencia por omissao
#define MAX_CONCURRENCY 0

// número de threads associadas à completion port
#define MAX_THREADS	10


// LowLevel Async actions
BOOL AsyncRead(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr);
BOOL AsyncWrite(HANDLE sd, LPVOID buffer, DWORD length, OVERLAPPED *ovr);
 
/* creator and device association functions   */
HANDLE CreateCompletionPort(int maxConcurrency);
BOOL CompletionPortAssociateHandle(HANDLE devHandle, ULONG_PTR completionKey);	 
VOID CreateThreadPool(HANDLE ioPort, int numThreads);
 
// Create and start the supporting completion port!
// must be called before any other infraestruture operation
VOID StartAsync();
 