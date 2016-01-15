#include "stdafx.h"

#include "AsyncOpers.h"

#define FILEDUMP_BUFFER_SIZE 128

typedef struct fileDumpOper {
	IOBaseOper base;	// trata-se de uma operação, embora composta
	BYTE buffer[FILEDUMP_BUFFER_SIZE];	// buffer de transferência
	LARGE_INTEGER filePos;				// posição corrente
} FileDumpOper, *PFileDumpOper;


VOID InitFileDumpOper(PFileDumpOper dfc, PCallback cb, LPVOID ctx) {
	// uma vez que se trata de uma operação composta, que não lida
	// directamente com devices, não tem device nem complete action associadas
	InitBase(&dfc->base, NULL, cb, ctx, NULL);
	dfc->filePos.QuadPart = 0;
}


// Por simplicidade, não h+a verificação de erro na operação
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
