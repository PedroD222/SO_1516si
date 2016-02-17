// PipePoster.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Pipe/Pipe.h"
#include "../PipeObj/PipeObj.h"

#define MSG_SIZE ATOMIC_RW
typedef BYTE MESSAGE[MSG_SIZE + 1];

/*-----------------------------------------------------------
* Auxiliary function
*   Fill the message with specified char
*------------------------------------------------------------*/
VOID FillMessage(MESSAGE m, char c) {
	for (int i = 0; i < MSG_SIZE; ++i) {
		m[i] = c;
	}
	m[MSG_SIZE] = 0;
}

#define NTRIES_TEST2 10000

UINT WINAPI ClientTest2(LPVOID arg) {
	
	//HANDLE p = PipeOpenWrite(NAMEPIPE);
	HANDLE p = PipeOpenWrite(NAMEPIPE);
	MESSAGE m;

	char currchar = 'A';
	// send NTRIES_TEST2 messages
	for (int n = 0; n < NTRIES_TEST2; ++n) {
		FillMessage(m, currchar);
		if (currchar == 'Z') currchar = 'A';
		else currchar++;

		PipeWrite(p, m, MSG_SIZE);
	}
	PipeClose(p);

	return 0;
}


VOID PipeTest_OneClient_OneServer() {

	HANDLE threads[2];
	DWORD testOk;


	printf("%-40s", "PipeTest_OneClient_OneServer test ...  ");

	//threads[0] = (HANDLE)_beginthreadex(NULL, 0, ServerTest2, ppipe, 0, NULL);
	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ClientTest2, NULL, 0, NULL);

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

/*
VOID PipeTest_TwoClients_OneServer() {
	PPIPE ppipe;
	HANDLE threads[3];
	DWORD testOk;

	printf("%-40s", "PipeTest_TwoClients_OneServer test ...  ");
	ppipe = PipeCreate();

	InitializeCriticalSection(&clientMutex);

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ServerTest3, ppipe, 0, NULL);
	threads[1] = (HANDLE)_beginthreadex(NULL, 0, ClientTest3, ppipe, 0, NULL);
	threads[2] = (HANDLE)_beginthreadex(NULL, 0, ClientTest3, ppipe, 0, NULL);

	for (int i = 0; i < 3; ++i) {

		WaitForSingleObject(threads[i], INFINITE);
		if (i == 0) { // server thread
			if (!GetExitCodeThread(threads[0], &testOk))
				testOk = 0;
		}
		CloseHandle(threads[i]);
	}
	printf("%s\n", testOk ? "PASS!" : "FAIL!");
}*/

// shared state for two cliente in the test
char currchar = 'A';
CRITICAL_SECTION clientMutex;


// number of send messages in each client
#define NTRIES_TEST3  10000

UINT WINAPI ClientTest3(LPVOID arg) {
	HANDLE p = PipeOpenWrite(NAMEPIPE);
	MESSAGE m;

	// send NTRIES messages
	for (int n = 0; n < NTRIES_TEST3; ++n) {
		EnterCriticalSection(&clientMutex);
		FillMessage(m, currchar);
		if (currchar == 'Z') currchar = 'A';
		else currchar++;
		LeaveCriticalSection(&clientMutex);

		PipeWrite(p, m, MSG_SIZE);

	}
	PipeClose(p);

	return 0;
}




VOID PipeTest_TwoClients_OneServer() {
	HANDLE threads[2];
	DWORD testOk;

	printf("%-40s", "PipeTest_TwoClients_OneServer test ...  ");

	InitializeCriticalSection(&clientMutex);

	//threads[0] = (HANDLE)_beginthreadex(NULL, 0, ServerTest3, ppipe, 0, NULL);
	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ClientTest3, "", 0, NULL);
	threads[1] = (HANDLE)_beginthreadex(NULL, 0, ClientTest3, "", 0, NULL);

	for (int i = 0; i < 2; ++i) {

		WaitForSingleObject(threads[i], INFINITE);
		if (i == 0) { // server thread
			if (!GetExitCodeThread(threads[0], &testOk))
				testOk = 0;
		}
		CloseHandle(threads[i]);
	}
	printf("%s\n", testOk ? "PASS!" : "FAIL!");
}


#define STREAM_SIZE 8192
BYTE input[STREAM_SIZE];
BYTE output[STREAM_SIZE];



BOOL CheckStreams() {
	for (INT i = 0; i < STREAM_SIZE; ++i) {
		if (input[i] != output[i]) return FALSE;
	}
	return TRUE;
}


VOID FillInputStream() {
	for (INT i = 0; i < STREAM_SIZE; ++i) input[i] = rand() % 256;
}


UINT WINAPI Sender(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenWrite(NAMEPIPE);

	INT i = 0;
	while (i < STREAM_SIZE) {
		PipeWrite(p, input + i++, 1);
	}

	// close pipe access via "p" handle
	PipeClose(p);
	return 0;
}

VOID PipeTest_Stream() {
	PPIPE ppipe;
	HANDLE threads[2];

	printf("%-40s", "PipeTest_Stream test ...  ");
	FillInputStream();

	//ppipe = PipeCreate(NAMEPIPE);

	//threads[0] = (HANDLE)_beginthreadex(NULL, 0, Receiver, ppipe, 0, NULL);
	threads[1] = (HANDLE)_beginthreadex(NULL, 0, Sender, "", 0, NULL);

	WaitForMultipleObjects(2, threads, TRUE, INFINITE);
	CloseHandle(threads[0]);
	CloseHandle(threads[1]);

	if (!CheckStreams()) {
		printf("FAIL!\n");
	}
	else {
		printf("PASS!\n");
	}
}




int _tmain(int argc, _TCHAR* argv[])
{
	//PPIPE create = PipeCreate(_T("pipe"));
	//HANDLE pipe = PipeOpenWrite(_T("pipe"));
	//PPIPE create = PipeCreate(NAMEPIPE);
	PipeTest_OneClient_OneServer();
	getchar();

	PipeTest_TwoClients_OneServer();
	getchar();

	PipeTest_Stream();
	getchar();

	return 0;
}

