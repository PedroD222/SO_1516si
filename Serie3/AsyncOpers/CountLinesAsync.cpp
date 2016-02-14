#include "stdafx.h"
#include "AsyncOpers.h"

BOOL CountLinesAsync(LPCTSTR fileIn, // pathname do ficheiro
	LPCSTR match, // string a procurar
	PCallback cb, // função a invocar na conclusão da operação
	LPVOID ctx) { // contexto (user) a passar à função de callback
	PIOAsyncDev dev = OpenAsync(fileIn);

	return FALSE;
}