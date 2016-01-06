/////////////////////////////////////////////////////////////////
//
// CCISEL 
// 2007-2011
//
// UThread library:
//   User threads supporting cooperative multithreading.
//
// Authors:
//   Carlos Martins, Jo�o Trindade, Duarte Nunes, Jorge Martins
// 

#include <crtdbg.h>
#include "UThreadInternal.h"

//////////////////////////////////////
//
// UThread internal state variables.
//

//
// The number of existing user threads.
//
static
ULONG NumberOfThreads;

//
// The sentinel of the circular list linking the user threads that are
// currently schedulable. The next thread to run is retrieved from the
// head of this list.
//
static
LIST_ENTRY ReadyQueue;

//////////////////////////////////////////////
static 
LIST_ENTRY AliveQueue;


static 
LIST_ENTRY WaitQueue;

static
LIST_ENTRY ContextQueue;
//////////////////////////////////////////////
//
// The currently executing thread.
//
#ifndef _WIN64
static
#endif
PUTHREAD RunningThread;



//
// The user thread proxy of the underlying operating system thread. This
// thread is switched back in when there are no more runnable user threads,
// causing the scheduler to exit.
//
static
PUTHREAD MainThread;

//
// Counter of Thread commutations
//
static
ULONG SwitchCount = 0;

////////////////////////////////////////////////
//
// Forward declaration of internal operations.
//

//
// The trampoline function that a user thread begins by executing, through
// which the associated function is called.
//
static
VOID InternalStart ();


#ifdef _WIN64
//
// Performs a context switch from CurrentThread to NextThread.
// In x64 calling convention CurrentThread is in RCX and NextThread in RDX.
//
VOID __fastcall  ContextSwitch64 (PUTHREAD CurrentThread, PUTHREAD NextThread);

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// In x64 calling convention  CurrentThread is in RCX and NextThread in RDX.
//
VOID __fastcall InternalExit64 (PUTHREAD Thread, PUTHREAD NextThread);

#define ContextSwitch ContextSwitch64
#define InternalExit InternalExit64

#else

static
VOID __fastcall ContextSwitch32 (PUTHREAD CurrentThread, PUTHREAD NextThread);

//
// Frees the resources associated with CurrentThread and switches to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX
// and NextThread in EDX.
//
static
VOID __fastcall InternalExit32 (PUTHREAD Thread, PUTHREAD NextThread);

#define ContextSwitch ContextSwitch32
#define InternalExit InternalExit32
#endif

////////////////////////////////////////
//
// UThread inline internal operations.
//

//
// Returns and removes the first user thread in the ready queue. If the ready
// queue is empty, the main thread is returned.
//
static
FORCEINLINE
PUTHREAD ExtractNextReadyThread () {
	return IsListEmpty(&ReadyQueue) 
		 ? MainThread 
		 : CONTAINING_RECORD(RemoveHeadList(&ReadyQueue), UTHREAD, Link);
}

//
// Schedule a new thread to run
//
static
FORCEINLINE
VOID Schedule () {
	PUTHREAD NextThread;
    NextThread = ExtractNextReadyThread();
	//RunningThread->State = READY;	//<--- caso seja deactivate esta afecta��o n�o vai fazer nada
	NextThread->State = RUNNING;
	SwitchCount++;
	ContextSwitch(RunningThread, NextThread);
}

///////////////////////////////
//
// UThread public operations.
//

//
// Initialize the scheduler.
// This function must be the first to be called. 
//
VOID UtInit() {
	InitializeListHead(&ReadyQueue);
	//New
	InitializeListHead(&AliveQueue);
	InitializeListHead(&WaitQueue);
	//
}


BOOL UtAlive(HANDLE t){
	PLIST_ENTRY curr = AliveQueue.Flink;
	while (curr != &AliveQueue){
		if (curr == &((PUTHREAD)t)->AliveLink){
			return TRUE;
		}
		curr = curr->Flink;
	}
	return FALSE;
}

BOOL UtJoin(HANDLE thread){
	PUTHREAD toJoin;
	if(!UtAlive(thread)) return FALSE;
	toJoin = (PUTHREAD)thread;
	//PUTHREAD running = UtSelf();
	RunningThread->State = BLOCKED;
	RunningThread->ToWaitFor = toJoin;
	InsertTailList(&WaitQueue, &RunningThread->WaitLink);
	UtDeactivate();
	RunningThread->State = RUNNING;
	return TRUE;
}

FLOAT UtStackUsage(HANDLE thread){
	PUTHREAD t = (PUTHREAD)thread;
	FLOAT semRes;
	semRes = (FLOAT)((sizeof(UTHREAD_CONTEXT)+sizeof(ULONG))) / (FLOAT)t->StackSz;
	return semRes * 100;
}


VOID UtDump(){
	PLIST_ENTRY curr = AliveQueue.Flink;
	FLOAT perc;
	while (curr != &AliveQueue){
		PUTHREAD t = CONTAINING_RECORD(curr, UTHREAD, AliveLink);
		//_tprintf(T("Handle: %p ---- Name: %s \n",*t,t->Name));
		printf("Handle: %p ---- Name: %s \n", t, t->Name);
		printf("State: %s \n", t->State);
		perc = UtStackUsage(t);
		printf("Stack filled percentage: %.4f \n",perc);
		curr = curr->Flink;
	}
}


//
// Cleanup all UThread internal resources.
//
VOID UtEnd() {
	/* (this function body was intentionally left empty) */
}

//
// Run the user threads. The operating system thread that calls this function
// performs a context switch to a user thread and resumes execution only when
// all user threads have exited.
//
VOID UtRun () {
	UTHREAD Thread; // Represents the underlying operating system thread.

	//
	// There can be only one scheduler instance running.
	//
	_ASSERTE(RunningThread == NULL);

	//
	// At least one user thread must have been created before calling run.
	//
	if (IsListEmpty(&ReadyQueue)) {
		return;
	}

	//
	// Switch to a user thread.
	//
	MainThread = &Thread;
	RunningThread = MainThread;
	Schedule();
 
	//
	// When we get here, there are no more runnable user threads.
	//
	_ASSERTE(IsListEmpty(&ReadyQueue));
	_ASSERTE(NumberOfThreads == 0);

	//
	// Allow another call to UtRun().
	//
	RunningThread = NULL;
	MainThread = NULL;
}




//
// Terminates the execution of the currently running thread. All associated
// resources are released after the context switch to the next ready thread.
//
VOID UtExit () {
	NumberOfThreads -= 1;	
	PLIST_ENTRY curr = WaitQueue.Flink;
	while (curr != &WaitQueue){
		PUTHREAD t = CONTAINING_RECORD(curr, UTHREAD, WaitLink);
		if (t->ToWaitFor == RunningThread){
			//RemoveEntryList(&t->AliveLink);
			RemoveEntryList(curr);
			t->State = READY;
			//InsertHeadList(&ReadyQueue, &t->AliveLink);
			//InsertTailList(&ReadyQueue, &((PUTHREAD)curr)->Link);
			InsertTailList(&ReadyQueue, &((PUTHREAD)t)->Link);
		}
		curr = curr->Flink;
	}
	InternalExit(RunningThread, ExtractNextReadyThread());
	_ASSERTE(!"Supposed to be here!");
}

//
// Relinquishes the processor to the first user thread in the ready queue.
// If there are no ready threads, the function returns immediately.
//
VOID UtYield () {
	if (!IsListEmpty(&ReadyQueue)) {
		InsertTailList(&ReadyQueue, &RunningThread->Link);
		Schedule();
	}
}

//
// Returns a HANDLE to the executing user thread.
//
HANDLE UtSelf () {
	return (HANDLE)RunningThread;
}




//
// Halts the execution of the current user thread.
//
VOID UtDeactivate() {
	Schedule();
}


//
// Places the specified user thread at the end of the ready queue, where it
// becomes eligible to run.
//
// New Features: When a new thread is created, it is placed in the alive queue
//
VOID UtActivate (HANDLE ThreadHandle) {
	InsertTailList(&ReadyQueue, &((PUTHREAD)ThreadHandle)->Link);
	InsertTailList(&AliveQueue, &((PUTHREAD)ThreadHandle)->AliveLink);
}

//
//Returns the number of UTrheads switches
//
ULONG UtGetSwitchCount(){
	return SwitchCount;
}

ULONG GetReadyQueueSize(){
	ULONG counter = 0;
	PLIST_ENTRY curr = ReadyQueue.Flink;
	while (curr != &ReadyQueue){
		counter++;
		curr = curr->Flink;
	}
	return counter;

}

///////////////////////////////////////
//
// Definition of internal operations.
//

//
// The trampoline function that a user thread begins by executing, through
// which the associated function is called.
//
VOID InternalStart () {
	RunningThread->State = RUNNING;
	RunningThread->Function(RunningThread->Argument);
	SwitchCount++;
	UtExit(); 
}
/*
VOID InternalStartWithParams(UT_FUNCTION function, UT_ARGUMENT argument){
	function(argument);
	UtExit();
}
*/
//
// Frees the resources associated with Thread..
//
VOID __fastcall CleanupThread (PUTHREAD Thread) {
	free(Thread->Stack);
	free(Thread);
}

//
// functions with implementation dependent of X86 or x64 platform
//

#ifndef _WIN64

//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreate32 (UT_FUNCTION Function, UT_ARGUMENT Argument) {
	PUTHREAD Thread;
	int in;
	int stackSz;
	//TCHAR ThreadName[20];
	char opt[3];
	LPSTR ThreadName;// = "";
	Thread = (PUTHREAD)malloc(sizeof (UTHREAD));

	//////To delete
	WCHAR ThreadNameBuff [20];
	char staSz[10];
	/*
	ThreadName = "Unnamed";
	Thread->Name = ThreadName;
	stackSz = STACK_SIZE;
	Thread->StackSz = stackSz;
	///////////////////////////////////
	*/
	printf("Creating new Thread. Do you wish to name it? [y/n]");

	for (;;){
		scanf("%s", opt);// getchar();
		if (strcmp(opt, "n") == 0){
			Thread->Name = "Unnamed";
			break;
		}
		else if (strcmp(opt, "y") == 0){
			printf("Enter Thread name: [20 chars]");
			scanf("%s", ThreadNameBuff);
			ThreadName = &ThreadNameBuff;
			Thread->Name = ThreadName;
			break;
		}
	}
	stackSz = 0;
	printf("Do you wish to specify the stack size? [y/n]");
	for (;;){
		scanf("%s", opt);//in = getchar();
		if (strcmp(opt, "n") == 0){
			stackSz = STACK_SIZE;
			break;
		}
		else if (strcmp(opt, "y") == 0){
			printf("Enter the size in KB: (above 32)");
			scanf("%s", staSz);
			stackSz = atoi(staSz);
			//stackSz = getchar();
			break;
		}
	}
	Thread->StackSz = stackSz;

	//
	// Dynamically allocate an instance of UTHREAD and the associated stack.
	//
	//Thread = (PUTHREAD) malloc(sizeof (UTHREAD));
	Thread->Stack = (PUCHAR)malloc(stackSz);
	_ASSERTE(Thread != NULL && Thread->Stack != NULL);

	//
	// Zero the stack for emotional confort.
	//
	memset(Thread->Stack, 0, stackSz);

	//
	// Memorize Function and Argument for use in InternalStart.
	//
	Thread->Function = Function;
	Thread->Argument = Argument;

	//
	// Map an UTHREAD_CONTEXT instance on the thread's stack.
	// We'll use it to save the initial context of the thread.
	//
	// +------------+
	// | 0x00000000 |    <- Highest word of a thread's stack space
	// +============+       (needs to be set to 0 for Visual Studio to
	// |  RetAddr   | \     correctly present a thread's call stack).
	// +------------+  |
	// |    EBP     |  |
	// +------------+  |
	// |    EBX     |   >   Thread->ThreadContext mapped on the stack.
	// +------------+  |
	// |    ESI     |  |
	// +------------+  |
	// |    EDI     | /  <- The stack pointer will be set to this address
	// +============+       at the next context switch to this thread.
	// |            | \
	// +------------+  |
	// |     :      |  |
	//       :          >   Remaining stack space.
	// |     :      |  |
	// +------------+  |
	// |            | /  <- Lowest word of a thread's stack space
	// +------------+       (Thread->Stack always points to this location).
	//

	Thread->ThreadContext = (PUTHREAD_CONTEXT) (Thread->Stack +
		STACK_SIZE - sizeof (ULONG) - sizeof (UTHREAD_CONTEXT));

	//
	// Set the thread's initial context by initializing the values of EDI,
	// EBX, ESI and EBP (must be zero for Visual Studio to correctly present
	// a thread's call stack) and by hooking the return address.
	// 
	// Upon the first context switch to this thread, after popping the dummy
	// values of the "saved" registers, a ret instruction will place the
	// address of InternalStart on EIP.
	//
	Thread->ThreadContext->EDI = 0x33333333;
	Thread->ThreadContext->EBX = 0x11111111;
	Thread->ThreadContext->ESI = 0x22222222;
	Thread->ThreadContext->EBP = 0x00000000;
	VOID(*RetAddr)(UT_ARGUMENT);

	Thread->ThreadContext->RetAddr = InternalStart;
	Thread->State = READY;
	//Thread->ThreadContext->RetAddr = InternalStartWithParams(Function, Argument);

	//InitializeListHead(&Thread->waiters);
	//
	// Ready the thread.
	//
	NumberOfThreads += 1;
	UtActivate((HANDLE)Thread);
	
	
	return (HANDLE)Thread;
}

//
// Performs a context switch from CurrentThread to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX and NextThread in EDX.
// __declspec(naked) directs the compiler to omit any prologue or epilogue.
//
__declspec(naked) 
VOID __fastcall ContextSwitch32 (PUTHREAD CurrentThread, PUTHREAD NextThread) {
	__asm {
		// Switch out the running CurrentThread, saving the execution context on the thread's own stack.   
		// The return address is atop the stack, having been placed there by the call to this function.
		//
		push	ebp
		push	ebx
		push	esi
		push	edi
		//
		// Save ESP in CurrentThread->ThreadContext.
		//
		mov		dword ptr [ecx].ThreadContext, esp
		//
		// Set NextThread as the running thread.
		//
		mov     RunningThread, edx
		//
		// Load NextThread's context, starting by switching to its stack, where the registers are saved.
		//
		mov		esp, dword ptr [edx].ThreadContext

		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		//
		// Jump to the return address saved on NextThread's stack when the function was called.
		//
		ret
	}
}



//
// Frees the resources associated with CurrentThread and switches to NextThread.
// __fastcall sets the calling convention such that CurrentThread is in ECX and NextThread in EDX.
// __declspec(naked) directs the compiler to omit any prologue or epilogue.
//
__declspec(naked)
VOID __fastcall InternalExit32 (PUTHREAD CurrentThread, PUTHREAD NextThread) {
	__asm {

		//
		// Set NextThread as the running thread.
		//
		mov     RunningThread, edx
		
		//
		// Load NextThread's stack pointer before calling CleanupThread(): making the call while
		// using CurrentThread's stack would mean using the same memory being freed -- the stack.
		//
		mov		esp, dword ptr [edx].ThreadContext

		call    CleanupThread

		//
		// Finish switching in NextThread.
		//
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret
	}
}

#else

//
// Creates a user thread to run the specified function. The thread is placed
// at the end of the ready queue.
//
HANDLE UtCreate64 (UT_FUNCTION Function, UT_ARGUMENT Argument) {
	PUTHREAD Thread;
	
	//
	// Dynamically allocate an instance of UTHREAD and the associated stack.
	//
	Thread = (PUTHREAD) malloc(sizeof (UTHREAD));
	Thread->Stack = (PUCHAR) malloc(STACK_SIZE);
	_ASSERTE(Thread != NULL && Thread->Stack != NULL);

	//
	// Zero the stack for emotional confort.
	//
	memset(Thread->Stack, 0, STACK_SIZE);

	//
	// Memorize Function and Argument for use in InternalStart.
	//
	Thread->Function = Function;
	Thread->Argument = Argument;

	//
	// Map an UTHREAD_CONTEXT instance on the thread's stack.
	// We'll use it to save the initial context of the thread.
	//
	// +------------+  <- Highest word of a thread's stack space
	// | 0x00000000 |    (needs to be set to 0 for Visual Studio to
	// +------------+      correctly present a thread's call stack).   
	// | 0x00000000 |  \
	// +------------+   |
	// | 0x00000000 |   | <-- Shadow Area for Internal Start 
	// +------------+   |
	// | 0x00000000 |   |
	// +------------+   |
	// | 0x00000000 |  /
	// +============+       
	// |  RetAddr   | \    
	// +------------+  |
	// |    RBP     |  |
	// +------------+  |
	// |    RBX     |   >   Thread->ThreadContext mapped on the stack.
	// +------------+  |
	// |    RDI     |  |
	// +------------+  |
	// |    RSI     |  |
	// +------------+  |
	// |    R12     |  |
	// +------------+  |
	// |    R13     |  |
	// +------------+  |
	// |    R14     |  |
	// +------------+  |
	// |    R15     | /  <- The stack pointer will be set to this address
	// +============+       at the next context switch to this thread.
	// |            | \
	// +------------+  |
	// |     :      |  |
	//       :          >   Remaining stack space.
	// |     :      |  |
	// +------------+  |
	// |            | /  <- Lowest word of a thread's stack space
	// +------------+       (Thread->Stack always points to this location).
	//

	Thread->ThreadContext = (PUTHREAD_CONTEXT) (Thread->Stack +
		STACK_SIZE -sizeof (UTHREAD_CONTEXT)-sizeof(ULONGLONG)*5);

	//
	// Set the thread's initial context by initializing the values of 
	// registers that must be saved by the called (R15,R14,R13,R12, RSI, RDI, RBCX, RBP)
	
	// 
	// Upon the first context switch to this thread, after popping the dummy
	// values of the "saved" registers, a ret instruction will place the
	// address of InternalStart on EIP.
	//
	Thread->ThreadContext->R15 = 0x77777777;
	Thread->ThreadContext->R14 = 0x66666666;
	Thread->ThreadContext->R13 = 0x55555555;
	Thread->ThreadContext->R12 = 0x44444444;	
	Thread->ThreadContext->RSI = 0x33333333;
	Thread->ThreadContext->RDI = 0x11111111;
	Thread->ThreadContext->RBX = 0x22222222;
	Thread->ThreadContext->RBP = 0x11111111;		
	Thread->ThreadContext->RetAddr = InternalStart;

	//
	// Ready the thread.
	//
	NumberOfThreads += 1;
	UtActivate((HANDLE)Thread);
	
	return (HANDLE)Thread;
}




#endif