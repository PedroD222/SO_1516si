#include "stdafx.h"
#include "AsyncOpers.h"

BOOL CountLinesAsync(LPCTSTR fileIn, // pathname do ficheiro
	LPCSTR match, // string a procurar
	PCallback cb, // fun��o a invocar na conclus�o da opera��o
	LPVOID ctx) { // contexto (user) a passar � fun��o de callback
	PIOAsyncDev dev = OpenAsync(fileIn);

	return FALSE;
}