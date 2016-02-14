// PipeReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Pipe/Pipe.h"
//#include "../PipeService/PipeService.h"
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




/*-----------------------------------------------------------
* Auxiliary function.
* check if all chars in message are equal
* If not it means the message was not atomically written or read
/*------------------------------------------------------------ - */
BOOL CheckMessageTest3(MESSAGE m) {
	char c = m[0];
	for (INT i = 1; i < MSG_SIZE; ++i) {
		if (m[i] != c)
			return FALSE;
	}
	return TRUE;
}

// server thread function used in second and third tests
UINT WINAPI ServerTest3(LPVOID arg) {
	//PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenRead(NAMEPIPE);
	MESSAGE m = { 0 };
	UINT ret = 1;

	// try for one thousand times
	while (PipeRead(p, m, MSG_SIZE) != 0) {
		if (!CheckMessageTest3(m)) {
			ret = 0;
			break;
		}
	}
	PipeClose(p);
	return ret;
}


VOID PipeTest_TwoClients_OneServer() {
	//PPIPE ppipe;
	HANDLE threads[1];
	DWORD testOk;

	printf("%-40s", "PipeTest_TwoClients_OneServer test ...  ");
	//ppipe = PipeCreate(NAMEPIPE);

	//InitializeCriticalSection(&clientMutex);

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ServerTest3, "", 0, NULL);
	//threads[1] = (HANDLE)_beginthreadex(NULL, 0, ClientTest3, ppipe, 0, NULL);
	//threads[2] = (HANDLE)_beginthreadex(NULL, 0, ClientTest3, ppipe, 0, NULL);

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



#define STREAM_SIZE 8192
BYTE input[STREAM_SIZE];
BYTE output[STREAM_SIZE];

UINT WINAPI Receiver(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;

	// get access to pipe for write
	HANDLE p = PipeOpenRead(NAMEPIPE);
	BYTE b;
	INT i = 0;

	while (PipeRead(p, &b, 1)) {
		output[i++] = b;
	}

	// close pipe access via "p" handle
	PipeClose(p);
	return 0;
}

BOOL CheckStreams() {
	for (INT i = 0; i < STREAM_SIZE; ++i) {
		if (input[i] != output[i]) return FALSE;
	}
	return TRUE;
}


VOID FillInputStream() {
	for (INT i = 0; i < STREAM_SIZE; ++i) input[i] = rand() % 256;
}




VOID PipeTest_Stream() {
	PPIPE ppipe;
	HANDLE threads[2];

	printf("%-40s", "PipeTest_Stream test ...  ");
	FillInputStream();

	//ppipe = PipeCreate();

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, Receiver, "", 0, NULL);
	//threads[1] = (HANDLE)_beginthreadex(NULL, 0, Sender, ppipe, 0, NULL);

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
	//HANDLE pipe = PipeOpenRead(_T("pipe"));
	//PPIPE create = PipeCreate(NAMEPIPE);
	PipeTest_OneClient_OneServer();
	getchar();
	PipeTest_TwoClients_OneServer();
	getchar();
	PipeTest_Stream();
	getchar();

	return 0;
}

