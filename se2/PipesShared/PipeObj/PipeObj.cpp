// PipeObj.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "PipeObj.h"
//#include "../Pipe/Pipe.h"
#include "../PipeService/PipeService.h"

HANDLE LaunchProcess(TCHAR *cmdLine) {
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (!CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		return NULL;
	}

	CloseHandle(pi.hThread);
	return pi.hProcess;
}

int _tmain(int argc, _TCHAR* argv[])
{
	PPIPE create = PipeCreate(NAMEPIPE);
	//HANDLE child2 = LaunchProcess(_T("PipeReader"));

	getchar();
	return 0;
}

