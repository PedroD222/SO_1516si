#pragma once

#include "CompletionPort.h"


#define BUFFER_SIZE 256

typedef struct iOBaseOper IOBaseOper, *PIOBaseOper;

// Representa o acesso asssíncrono a um ficheiro.
// O modelo não admite mais do que uma operação assíncrona 
// sobre um ficheiro num dado momento, pelo que uma única estrutura overlapped
// é suficiente.
// Por simplicidade, este invariante não é verificado na implementação fornecida.
// A associação a uma operação específica só existe enquanto a operação está em curso,
// podendo uma operação envolver várias leituras/escritas sobre o ficheiro,
// mas sempre em série.
// O paralelismo (de I/O) implícito no modelo é expresso na ocorrência
// de operações simultâneas sobre diferentes ficheiros.
typedef struct IOAsyncDev {
	OVERLAPPED ovr;		// representa (para o windows) a operação de I/O em curso
	HANDLE dev;			// handle para o ficheiro associado
	PIOBaseOper oper;	// refere a operação em curso
} IOAsyncDev, *PIOAsyncDev;


// definições associadas a uma operação assíncrona

// Assinatura da função que implementa a máquina de estados necessária 
// a uma determinada operação. Na sua forma mais simples consiste 
// apenas na invocação do callback do utilizador
typedef VOID(*CompleteAction)(PIOBaseOper aop, int transferedBytes);

// Assinatura da função de callback especificada pelo cliente da operação
// o contexto recebido é na verdade um ponteiro para a operação
// a partir dele se podem obter várias componentes da mesma
// através das funções acessoras:
// BOOL OperSuccess(LPVOID ctx)  - obtem o estado de sucesso/insucesso da operação
// LPVOID CtxGetUserContext(LPVOID ctx) - obtém o contexto especificado pelo cliente
// DWORD CtxGetTransferedBytes(LPVOID ctx) - obtém o número de bytes tarnsferidos
//   
// Poderão ser definidos acessores especíicos de determinadas operações
//
typedef VOID(*PCallback)(PIOAsyncDev dev, LPVOID ctx);

// estrutura base de qualquer operação assíncrona
struct iOBaseOper{
	PIOAsyncDev aHandle;			// ficheiro (device) associado
	PCallback userCb;				// função a invocar no final da operação
	LPVOID uCtx;					// contexto a passar à função (definidos pelo cliente)
	CompleteAction completeAction;	// representa a máquina de estados da operação
	DWORD transferedBytes;			// bytes transferidos de/para o device na operação
	BOOL success;					// indica o estado (sucesso/insucesdo) da operação
};


// Creators/destructor and basic operations

PIOAsyncDev OpenAsync(LPCTSTR fileName);
PIOAsyncDev CreateAsync(LPCTSTR fileName);
VOID CloseAsync(PIOAsyncDev ah);


VOID FileCopyAsync(LPCTSTR fileIn, LPCTSTR fileOut, PCallback cb, LPVOID ctx);
BOOL CopyFile2Async(LPCTSTR file, LPCTSTR fileOut, PCallback cb, LPVOID ctx); 
BOOL ReadAsync(PIOAsyncDev ah, LARGE_INTEGER offset, LPVOID buffer, DWORD length, PCallback cb, LPVOID ctx);
VOID CopyFolderAsync(LPCTSTR folderIn, LPCTSTR folderOut, PCallback cb, LPVOID ctx);
BOOL FileDumpAsync(LPCTSTR file, PCallback cb, LPVOID ctx);

BOOL WriteAsync(PIOAsyncDev asyncDev, LARGE_INTEGER offset, LPVOID buffer, DWORD length, PCallback cb, LPVOID ctx);

// acessores

FORCEINLINE
BOOL OperSuccess(LPVOID ctx) { return ((PIOBaseOper)ctx)->success; }

FORCEINLINE
VOID OperSetSuccess(PIOBaseOper oper, BOOL success) { oper->success = success; }

FORCEINLINE
VOID OperSetError(PIOBaseOper oper) { OperSetSuccess(oper, FALSE); }

FORCEINLINE
VOID OperSetOK(PIOBaseOper oper) { OperSetSuccess(oper, TRUE); }

LPVOID CtxGetUserContext(LPVOID ctx);
DWORD CtxGetTransferedBytes(LPVOID ctx);


// auxiliary functions

void InvokeCallbackAndReleaseOper(PIOBaseOper op);

FORCEINLINE
VOID InitBase(PIOBaseOper aop, PIOAsyncDev ah, PCallback cb, LPVOID uctx, CompleteAction ca) {
	aop->aHandle = ah;
	aop->userCb = cb;
	aop->uCtx = uctx;
	aop->completeAction = ca;
	aop->success = TRUE;
	if (ah != NULL) ah->oper = aop;
	aop->transferedBytes = 0;
}