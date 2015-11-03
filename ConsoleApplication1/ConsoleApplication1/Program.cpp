// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void query_show_dll_process(HANDLE process) {
	MEMORY_BASIC_INFORMATION mbi;
	PBYTE adress = NULL;
	DWORD err = 0;
	wchar_t name[MAX_PATH];
	DWORD sizenamedll;
	//HMODULE hMod;
	while (VirtualQueryEx(process, adress, &mbi, sizeof(mbi))) {
		if (mbi.State == MEM_COMMIT) {
			/*err = GetLastError();
			if (err != 0)
				_tprintf(_T("ERROR: %d \n"), err);*/
			//hMod = (HMODULE)mbi.AllocationBase;
			//sem if apanha varias vezes a mesma dll
			if (mbi.BaseAddress == mbi.AllocationBase) {
				sizenamedll = GetModuleFileNameEx(process, (HMODULE)mbi.AllocationBase, name, _countof(name));
				if (sizenamedll != 0)
					_tprintf(_T("Endereço Base: %d - Tamanho da Regiao %d - DLL: %s \n"), mbi.AllocationBase, mbi.RegionSize, name);
				/*else {
					_tprintf(_T("ERROR: %d \n"), GetLastError());
				}*/
			}
		}
		adress += mbi.RegionSize;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		_tprintf(_T("Usage: program <pid>\n"));
		getchar();
		return 1;
	}
	DWORD pid = _ttoi(argv[1]);
	_tprintf(_T("PID: %d \n"), pid);
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (process == NULL) {
		_tprintf(_T("Error on open process: %d \n"), GetLastError());
		getchar();
		return 1;
	}
	/*open process
	virtualqueryex iterar sobre todas as regiões, regiao committed, começar na 0,
	getModuleFilenameEX handle process,
						hmodule - corresponde ao endereço base de mapeamento da dll via virtual query*/
	query_show_dll_process(process);//aparece exe é suposto??
	int cl = CloseHandle(process);
	if (cl == 0)
		_tprintf(_T("ERROR: %d \n"), GetLastError());
	_tprintf(_T("Pressionar qualquer tecla para sair\n"));
	getchar();
    return 0;
}

