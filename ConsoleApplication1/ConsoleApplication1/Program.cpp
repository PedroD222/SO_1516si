// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void show_error(DWORD err) {
	if (err ) {
		_tprintf(_T("ERROR : %d \n"), err);
	}
}

void query_show_dll_process(HANDLE process) {
	MEMORY_BASIC_INFORMATION mbi;
	PBYTE adress = NULL;
	DWORD err = 0;
	wchar_t name[MAX_PATH];
	DWORD sizenamedll;
	
	while (VirtualQueryEx(process, adress, &mbi, sizeof(mbi))) {
		
		if (mbi.State == MEM_COMMIT) {
			
			//filtra caso dll ocupa mais de 1 região
			if (mbi.BaseAddress == mbi.AllocationBase) {
				sizenamedll = GetModuleFileNameEx(process, (HMODULE)mbi.AllocationBase, name, _countof(name));
				
				if (sizenamedll != 0)
					_tprintf(_T("Endereço Base: 0x%0x - Tamanho da Regiao: %d - DLL: %s \n"), mbi.AllocationBase, mbi.RegionSize, name);
				/*else
				{
					_tprintf(_T("ERROR GETMODULEFILENAMEEX \n"));
					show_error(GetLastError());
				}*/
			}
		}
		adress += mbi.RegionSize;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		_tprintf(_T("PID Missing. Usage: program <pid>\n"));
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
	
	query_show_dll_process(process);
	int cl = CloseHandle(process);
	if (cl == 0)
		show_error( GetLastError());
	_tprintf(_T("Press and key to exit\n"));
	getchar();
    return 0;
}

