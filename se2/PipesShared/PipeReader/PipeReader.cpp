// PipeReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../PipeService/PipeService.h"

int _tmain(int argc, _TCHAR* argv[])
{
	PPIPE pipe = PipeOpenRead(NULL);
	return 0;
}

