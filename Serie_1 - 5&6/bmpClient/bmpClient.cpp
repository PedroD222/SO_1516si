// bmpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	RGBTRIPLE orig;
	orig.rgbtBlue = 255;
	orig.rgbtGreen = 255;
	orig.rgbtRed = 255;

	RGBTRIPLE dest;
	dest.rgbtBlue = 128;
	dest.rgbtGreen = 128;
	dest.rgbtRed = 128;
	LPCTSTR input = (LPCTSTR)"mono.bmp";
	LPCTSTR output = (LPCTSTR)"mono_copy.bmp";
	BOOL res = ChangeBitMapTo(input, output, orig, dest);
	return 0;
}

