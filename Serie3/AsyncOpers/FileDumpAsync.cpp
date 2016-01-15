#include "stdafx.h"

#include "AsyncOpers.h"

#define FILEDUMP_BUFFER_SIZE 128

typedef struct fileDumpOper {
	IOBaseOper base;	// trata-se de uma opera��o, embora composta
	BYTE buffer[FILEDUMP_BUFFER_SIZE];	// buffer de transfer�ncia
	LARGE_INTEGER filePos;				// posi��o corrente
} FileDumpOper, *PFileDumpOper;


VOID InitFileDumpOper(PFileDumpOper dfc, PCallback cb, LPVOID ctx) {
	// uma vez que se trata de uma opera��o composta, que n�o lida
	// directamente com devices, n�o tem device nem complete action associadas
	InitBase(&dfc->base, NULL, cb, ctx, NULL);
	dfc->filePos.QuadPart = 0;
}


// Por simplicidade, n�o h+a verifica��o de erro na opera��o
VOID FileDumpCallback(PIOAsyncDev ad, LPVOID ctx) {
	PFileDumpOper dfc = (PFileDumpOper)CtxGetUserContext(ctx);
	DWORD transferedBytes = CtxGetTransferedBytes(ctx);

	if (transferedBytes == 0) { // the copy is done
		InvokeCallbackAndReleaseOper(&dfc->base);
		CloseAsync(ad);
		return;
	}

	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), dfc->buffer, transferedBytes,
		NULL, NULL);
	LARGE_INTEGER posAux = dfc->filePos;
	dfc->filePos.QuadPart += transferedBytes;
	ReadAsync(ad, dfc->filePos, dfc->buffer, FILEDUMP_BUFFER_SIZE, FileDumpCallback, dfc);
}

BOOL FileDumpAsync(LPCTSTR file, PCallback cb, LPVOID ctx) {

	PIOAsyncDev adIn = OpenAsync(file);
	if (adIn == NULL) {
		printf("can't open file, error %d!\n", GetLastError());
		return FALSE;
	}
	PFileDumpOper dfc = (PFileDumpOper)malloc(sizeof(FileDumpOper));
	InitFileDumpOper(dfc, cb, ctx);
	ReadAsync(adIn, dfc->filePos, dfc->buffer, FILEDUMP_BUFFER_SIZE, FileDumpCallback, dfc);
	return TRUE;
}
