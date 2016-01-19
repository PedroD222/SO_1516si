#pragma once

#include "CompletionPort.h"


#define BUFFER_SIZE 256

typedef struct iOBaseOper IOBaseOper, *PIOBaseOper;

// Representa o acesso asss�ncrono a um ficheiro.
// O modelo n�o admite mais do que uma opera��o ass�ncrona 
// sobre um ficheiro num dado momento, pelo que uma �nica estrutura overlapped
// � suficiente.
// Por simplicidade, este invariante n�o � verificado na implementa��o fornecida.
// A associa��o a uma opera��o espec�fica s� existe enquanto a opera��o est� em curso,
// podendo uma opera��o envolver v�rias leituras/escritas sobre o ficheiro,
// mas sempre em s�rie.
// O paralelismo (de I/O) impl�cito no modelo � expresso na ocorr�ncia
// de opera��es simult�neas sobre diferentes ficheiros.
typedef struct IOAsyncDev {
	OVERLAPPED ovr;		// representa (para o windows) a opera��o de I/O em curso
	HANDLE dev;			// handle para o ficheiro associado
	PIOBaseOper oper;	// refere a opera��o em curso
} IOAsyncDev, *PIOAsyncDev;


// defini��es associadas a uma opera��o ass�ncrona

// Assinatura da fun��o que implementa a m�quina de estados necess�ria 
// a uma determinada opera��o. Na sua forma mais simples consiste 
// apenas na invoca��o do callback do utilizador
typedef VOID(*CompleteAction)(PIOBaseOper aop, int transferedBytes);

// Assinatura da fun��o de callback especificada pelo cliente da opera��o
// o contexto recebido � na verdade um ponteiro para a opera��o
// a partir dele se podem obter v�rias componentes da mesma
// atrav�s das fun��es acessoras:
// BOOL OperSuccess(LPVOID ctx)  - obtem o estado de sucesso/insucesso da opera��o
// LPVOID CtxGetUserContext(LPVOID ctx) - obt�m o contexto especificado pelo cliente
// DWORD CtxGetTransferedBytes(LPVOID ctx) - obt�m o n�mero de bytes tarnsferidos
//   
// Poder�o ser definidos acessores espec�icos de determinadas opera��es
//
typedef VOID(*PCallback)(PIOAsyncDev dev, LPVOID ctx);

// estrutura base de qualquer opera��o ass�ncrona
struct iOBaseOper{
	PIOAsyncDev aHandle;			// ficheiro (device) associado
	PCallback userCb;				// fun��o a invocar no final da opera��o
	LPVOID uCtx;					// contexto a passar � fun��o (definidos pelo cliente)
	CompleteAction completeAction;	// representa a m�quina de estados da opera��o
	DWORD transferedBytes;			// bytes transferidos de/para o device na opera��o
	BOOL success;					// indica o estado (sucesso/insucesdo) da opera��o
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