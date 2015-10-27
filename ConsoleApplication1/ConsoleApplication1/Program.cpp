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
	PBYTE adress = 0x0;
	wchar_t name[256];
	DWORD sizenamedll;
	while (VirtualQueryEx(proc, adress, &mbi, sizeof(mbi))) {
		if (mbi.State == MEM_COMMIT) {
			sizenamedll = GetModuleFileNameEx(proc, (HMODULE)mbi.AllocationBase,  name, 256);
			if (sizenamedll != 0)
				_tprintf(_T("Endereço Base: %d - %d - %s \n"), mbi.AllocationBase, mbi.RegionSize, name);
		}
		adress += mbi.RegionSize;
	}
	CloseHandle(proc);
	_tprintf(_T("Pressionar qualquer tecla para sair\n"));
	getchar();
    return 0;
}

