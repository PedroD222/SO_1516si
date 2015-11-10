// bmpchange.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
/*
int change(LPCSTR in, LPCSTR out, byte or, byte og, byte ob, byte nr, byte ng, byte nb)
{
	OFSTRUCT ofstruct;

	BOOL copyRes = CopyFile(in, out, FALSE);
	if (copyRes == false){
		return -1;
	}

	HANDLE file = (HANDLE)OpenFile(out, &ofstruct, OF_READWRITE);

	HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, 0, NULL);

	void * mapView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	byte * st = (byte*)mapView + 10;
	byte * size = (byte*)mapView + 2;

	byte * end = st + *((int*)size);

	for (int i = 0; i < (int)(*size); i+=3)
	{

	}
	return 1;
}*/

int change(LPCSTR in, LPCSTR out, byte or, byte og, byte ob, byte nr, byte ng, byte nb)
{
	OFSTRUCT ofstruct;
	//CreateFile(out, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD err = GetLastError();
	BOOL copyRes = CopyFile(in, out, FALSE);
	err = GetLastError();
	if (copyRes == false){
		return -1;
	}

	HANDLE file = (HANDLE)OpenFile(out, &ofstruct, OF_READWRITE);
	err = GetLastError();
	HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, 0, NULL);
	err = GetLastError();
	void * mapView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	err = GetLastError();
	byte * st = (byte*)mapView + 10;
	byte * size = (byte*)mapView + 2;
	int szVal = (int)(*((int*)size));
	byte * end = st + *((int*)size);
	byte * begin = ((byte*)mapView) + 0xD + *((int*)st) - 1;
	for (int i = 0; i < szVal; i += 3)
	{
		byte * b = begin + i;
		if (*b == or){
			*b = nr;
		}
		if (*(b + 1) == og){
			*(b + 1) = ng;
		}
		if (*(b + 2) == ob){
			*(b + 2) = nb;
		}
	}
	return 1;

}

int _tmain(int argc, _TCHAR* argv[])
{
	/*
	if (argc < 10)
	{
		//Make a printf
		return 0;
	}*/

	byte or = _ttoi(argv[2]);
	byte og = _ttoi(argv[3]);
	byte ob = _ttoi(argv[4]);
	
	byte nr = _ttoi(argv[6]);
	byte ng = _ttoi(argv[7]);
	byte nb = _ttoi(argv[8]);
	//int ngi = _ttoi(ng);
	LPCSTR input = argv[9];// "mono.bmp";
	LPCSTR output = argv[10];// "mono_copy.bmp";
	change(input, output, or, og, ob, nr, ng, nb);
	return 0;
}

