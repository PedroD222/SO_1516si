// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		_tprintf(_T("Usage: program <pid>\n"));
		getchar();
		return 1;
	}
	DWORD pid = _ttoi(argv[1]);
	_tprintf(_T("PID: %d \n"), pid);

	HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (proc == NULL) {
		_tprintf(_T("ERROR: %d \n"), GetLastError());
		getchar();
		return 1;
	}
	/*open process
	virtualqueryex iterar sobre todas as regiões, regiao committed, começar na 0,
	getModuleFilenameEX handle process,
						hmodule - corresponde ao endereçoi base de mapeamento da dll via virtual query*/
	MEMORY_BASIC_INFORMATION mbi;
	PBYTE adress = NULL;
	DWORD err = 0;
	wchar_t name[256];
	DWORD sizenamedll;
	HMODULE hMod;
	while (VirtualQueryEx(proc, adress, &mbi, sizeof(mbi))) {
		/*err = GetLastError();
		if (err != 0)
			_tprintf(_T("ERROR: %d \n"), err);*/
		hMod = (HMODULE)mbi.AllocationBase;
		if (mbi.State == MEM_FREE) {
			hMod = (HMODULE)mbi.BaseAddress;
			mbi.AllocationBase = mbi.BaseAddress;
		}
			
		if (mbi.AllocationBase != NULL && mbi.BaseAddress == mbi.AllocationBase && mbi.State == MEM_COMMIT) {
			sizenamedll = GetModuleFileNameEx(proc, hMod, name, 256);
			if (sizenamedll != 0)
				_tprintf(_T("Endereço Base: %d - Regiao %d - DLL: %s \n"), mbi.AllocationBase, mbi.RegionSize, name);
			/*else {
				_tprintf(_T("ERROR: %d \n"), GetLastError());
			}*/
		}
		adress += mbi.RegionSize;
	}
	int cl = CloseHandle(proc);
	if (cl == 0)
		_tprintf(_T("ERROR: %d \n"), GetLastError());
	_tprintf(_T("Pressionar qualquer tecla para sair\n"));
	getchar();
    return 0;
}

