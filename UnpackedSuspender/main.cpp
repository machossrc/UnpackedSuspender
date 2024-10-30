#include "StdAfx.h"
#include "Memory/Memory.hpp"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage : UnpackedSuspender.exe <process_name>" << endl;
		return 0;
	}

	printf("Wait For %s ...\n", argv[1]);

	CMemory* pMemory = new CMemory(argv[1]);
	pMemory->Work();
	delete pMemory;
}