// bmpDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "bmpDLL.h"



BOOL WINAPI ChangeBitMapTo(LPCTSTR srcBitmap, LPCTSTR dstBitmap, RGBTRIPLE oldColour, RGBTRIPLE newColour){

	OFSTRUCT ofstruct;

	BOOL copyRes = CopyFile(srcBitmap, dstBitmap, FALSE);
	DWORD err = GetLastError();
	if (copyRes == false){
		return false;
	}

	HANDLE file = CreateFile(dstBitmap, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	err = GetLastError();
	if (err != 0)return false;
	HANDLE fileMapping = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, 0, NULL);
	err = GetLastError();
	if (err != 0)return false;
	void * mapView = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	err = GetLastError();
	if (err != 0)return false;

	BYTE * st = (BYTE*)mapView + 10;
	BYTE * size = (BYTE*)mapView + 2;
	int szVal = (int)(*((int*)size));
	BYTE * end = st + *((int*)size);
	BYTE * begin = ((BYTE*)mapView) + 0xD + *((int*)st) - 1;
	for (int i = 0; i < szVal; i += 3)
	{
		BYTE * b = begin + i;
		if (*b == oldColour.rgbtRed){
			*b = newColour.rgbtRed;
		}
		if (*(b + 1) == oldColour.rgbtGreen){
			*(b + 1) = newColour.rgbtGreen;
		}
		if (*(b + 2) == oldColour.rgbtBlue){
			*(b + 2) = newColour.rgbtBlue;
		}
	}
	return true;

}