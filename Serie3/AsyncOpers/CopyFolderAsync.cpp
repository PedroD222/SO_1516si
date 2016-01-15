#include "stdafx.h"
#include "AsyncOpers.h"

typedef struct copyFolderOper {
	IOBaseOper base;		//	Continua ser uma opera��o, embora suportada por outras
	LONG remainingFiles;	//  c�pias restantes
} CopyFolderOper, *PCopyFolderOper;


VOID CopyCompletedCallback(PIOAsyncDev ad, LPVOID ctx) {
	PCopyFolderOper cpOper = (PCopyFolderOper)CtxGetUserContext(ctx);

	// propagar o erro de uma c�pia individual para a opera��o
	// de c�pia de folder
	// O processo � fraco, apenas se sabe que houve um erro.
	if (!OperSuccess(ctx))
		OperSetError(&cpOper->base);
    if (InterlockedDecrement(&cpOper->remainingFiles) == 0)
		InvokeCallbackAndReleaseOper(&cpOper->base);
}


VOID InitCopyFolderOper(PCopyFolderOper cpOper, PCallback cb, LPVOID ctx) {
	// uma vez que se trata de uma opera��o composta, que n�o lida
	// directamente com devices, n�o tem device nem complete action associadas
	InitBase(&cpOper->base, NULL, cb, ctx, NULL);
	cpOper->remainingFiles = 1;
}

VOID CopyFolderAsync(LPCTSTR folderIn, LPCTSTR folderOut, PCallback cb, LPVOID ctx) {
	HANDLE iterator;
	WIN32_FIND_DATA fileData;
	TCHAR buffer1[MAX_PATH], buffer2[MAX_PATH];		// auxiliary buffers
	PCopyFolderOper cpf = (PCopyFolderOper)malloc(sizeof(CopyFolderOper));

	InitCopyFolderOper(cpf, cb, ctx);
	// the buffer is needed to define a match string that guarantees 
	// a priori selection for all files
	_stprintf_s(buffer1, _T("%s/%s"), folderIn, _T("*.*"));

	// start iteration
	if ((iterator = FindFirstFile(buffer1, &fileData)) == INVALID_HANDLE_VALUE) {
		OperSetError(&cpf->base);	// for�a estado de erro n aopera��o
		InvokeCallbackAndReleaseOper(&cpf->base);
		return;
	}
	 
	if (!CreateDirectory(folderOut, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		OperSetError(&cpf->base); // for�a estado de erro n aopera��o
		InvokeCallbackAndReleaseOper(&cpf->base);
		FindClose(iterator);
		return;
	}
  
	// process directory entries
	do {
		if (fileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			InterlockedIncrement(&cpf->remainingFiles);
			_stprintf_s(buffer1, _T("%s/%s"), folderIn, fileData.cFileName);
			_stprintf_s(buffer2, _T("%s/%s"), folderOut, fileData.cFileName);
			FileCopyAsync(buffer1, buffer2, CopyCompletedCallback, cpf);
		}
	} while (FindNextFile(iterator, &fileData));
	FindClose(iterator);
	
	if (InterlockedDecrement(&cpf->remainingFiles)== 0)
		InvokeCallbackAndReleaseOper(&cpf->base);	 
}

