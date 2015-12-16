// Serie2PipesTests.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "../serie2pipes/serie2pipes.h"



// definitions used in second and third tests
#define MSG_SIZE ATOMIC_RW
typedef BYTE MESSAGE[MSG_SIZE + 1];


//
// TEST 1
//
// Description: pipe as a stream of bytes - one sender, one receiver
//              for string copy
//

#define STREAM_SIZE 8192
BYTE input[STREAM_SIZE];
BYTE output[STREAM_SIZE];

/*-----------------------------------------------------------
* Auxiliary function.
* check if all chars in "inout" and "output" streams are equal.
* If not it means the copy was not corrcetly done
/*------------------------------------------------------------ - */
BOOL CheckStreams() {
	for (INT i = 0; i < STREAM_SIZE; ++i) {
		if (input[i] != output[i]) return FALSE;
	}
	return TRUE;
}

/*----------------------------------------------------
 * Random initialization of "input" string in test
 *----------------------------------------------------*/
VOID FillInputStream() {
	for (INT i = 0; i < STREAM_SIZE; ++i) input[i] = rand() % 256;
}

// receiver thread in the test
UINT WINAPI Receiver(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;

	// get access to pipe for write
	HANDLE p = PipeOpenRead(ppipe);
	BYTE b;
	INT i = 0;

	while (PipeRead(p, &b, 1)) {
		output[i++] = b;
	}

	// close pipe access via "p" handle
	PipeClose(p);
	return 0;
}


//sender thread in the test
UINT WINAPI Sender(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenWrite(ppipe);
	
	INT i = 0;
	while (i < STREAM_SIZE) {
		PipeWrite(p, input + i++, 1);
	}

	// close pipe access via "p" handle
	PipeClose(p);
	return 0;
}

/*--------------------------------------------------------
* Test for pipes
* scenario: just one client and one server 
*           pipe is used has a stream of bytes used to copy a string
*           sender to receiver      
*--------------------------------------------------------*/
VOID PipeTest_Stream() {
	PPIPE ppipe;
	HANDLE threads[2];

	printf("%-40s", "PipeTest_Stream test ...  ");
	FillInputStream();

	ppipe = PipeCreate();

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, Receiver, ppipe, 0, NULL);
	threads[1] = (HANDLE)_beginthreadex(NULL, 0, Sender, ppipe, 0, NULL);

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



//
// TEST 2
//
// Description:
//  now the pipe is treated as a stream of messages
//  message order and content is checked on server (receiver)
//




/*-------------------------------------------------------------
 * Auxiliary function
 *   Check the content of a received message
 *------------------------------------------------------------*/
BOOL CheckMessageTest2(MESSAGE m, char c) {
	for (int i = 0; i < MSG_SIZE; ++i) {
		if (m[i] != c) return FALSE;
	}
	return TRUE;
}

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

// server thread function used in second and third tests
UINT WINAPI ServerTest2(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenRead(ppipe);
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


#define NTRIES_TEST2 10000

UINT WINAPI ClientTest2(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenWrite(ppipe);
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

/*--------------------------------------------------------
 * Test for pipes
 * scenario: just one client and one server 
 *           client sends NTRIES messages and server check each
 *           message received        
 *--------------------------------------------------------*/
VOID PipeTest_OneClient_OneServer() {
	PPIPE ppipe;
	HANDLE threads[2];
	DWORD testOk;


	printf("%-40s", "PipeTest_OneClient_OneServer test ...  ");
	
	ppipe = PipeCreate();

	threads[0] = (HANDLE)_beginthreadex(NULL, 0, ServerTest2, ppipe, 0, NULL);
	threads[1] = (HANDLE)_beginthreadex(NULL, 0, ClientTest2, ppipe, 0, NULL);
	
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

//
// TEST 3
// Description:
//    two clients and one server.
//    client sends NTRIES messages each and server check each message received
//			


/*-----------------------------------------------------------
* Auxiliary function.
* check if all chars in message are equal
* If not it means the message was not atomically written or read
/*------------------------------------------------------------ - */
BOOL CheckMessageTest3(MESSAGE m) {
	char c = m[0];
	for (INT i = 1; i < MSG_SIZE; ++i) {
		if (m[i] != c) return FALSE;
	}
	return TRUE;
}

// server thread function used in second and third tests
UINT WINAPI ServerTest3(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenRead(ppipe);
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


// shared state for two cliente in the test
char currchar = 'A';
CRITICAL_SECTION clientMutex;

// number of send messages in each client
#define NTRIES_TEST3  10000

UINT WINAPI ClientTest3(LPVOID arg) {
	PPIPE ppipe = (PPIPE)arg;
	HANDLE p = PipeOpenWrite(ppipe);
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

/*---------------------------------------------------------------
* Test for pipes
* scenario: two clients and one server
*           client sends NTRIES messages each and server check each
*           message received
*			each message must be atomically written for test succeeed
*---------------------------------------------------------------*/
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
}


int _tmain(int argc, _TCHAR* argv[])
{
	PipeTest_OneClient_OneServer();
	PipeTest_TwoClients_OneServer();
	PipeTest_Stream();
	return 0;
}

