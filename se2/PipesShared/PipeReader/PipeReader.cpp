// PipeReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Pipe/Pipe.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//PPIPE create = PipeCreate(_T("pipe"));
	HANDLE pipe = PipeOpenRead(_T("pipe"));
	return 0;
}

