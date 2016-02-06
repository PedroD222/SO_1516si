// PipeReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Pipe/Pipe.h"
#include "../PipeObj/PipeObj.h"

#define MSG_SIZE ATOMIC_RW
typedef BYTE MESSAGE[MSG_SIZE + 1];

BOOL CheckMessageTest2(MESSAGE m, char c) {
	for (int i = 0; i < MSG_SIZE; ++i) {
		if (m[i] != c) return FALSE;
	}
	return TRUE;
}

UINT WINAPI ServerTest2(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenRead(NAMEPIPE);

	MESSAGE m = { 0 };
	char currchar = 'A';
	UINT ret = 1;

	// try for one thousand times
	while (PipeRead(p, m, MSG_SIZE) != 0) {
		if (!CheckMessageTest2(m, currchar)) {
			ret = 0;
			break;
		}

		if (currchar == 'Z') currchar = 'A';
		else currchar++;
	}
	PipeClose(p);
	return ret;
}

VOID PipeTest_OneClient_OneServer() {
	
	HANDLE threads[2];
	DWORD testOk;


	printf("%-40s", "PipeTest_OneClient_OneServer test ...  ");

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ServerTest2, NULL, 0, NULL);
	//threads[1] = (HANDLE)_beginthreadex(NULL, 0, ClientTest2, ppipe, 0, NULL);

	for (int i = 0; i < 1; ++i) {
		WaitForSingleObject(threads[i], INFINITE);
		if (i == 0) { // server thread
			if (!GetExitCodeThread(threads[0], &testOk))
				testOk = 0;
		}
		CloseHandle(threads[i]);
	}
	printf("%s\n", testOk ? "PASS!" : "FAIL!");
}

int _tmain(int argc, _TCHAR* argv[])
{
	//PPIPE create = PipeCreate(_T("pipe"));
	//HANDLE pipe = PipeOpenRead(_T("pipe"));
	//PPIPE create = PipeCreate(NAMEPIPE);
	//PipeTest_OneClient_OneServer();
	//
	////PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenRead(NAMEPIPE);

	MESSAGE m = { 0 };
	char currchar = 'A';
	UINT ret = 1;

	// try for one thousand times
	while (PipeRead(p, m, MSG_SIZE) != 0) {
		if (!CheckMessageTest2(m, currchar)) {
			ret = 0;
			break;
		}

		if (currchar == 'Z') currchar = 'A';
		else currchar++;
	}
	PipeClose(p);
	
	getchar();
	return 0;
}

