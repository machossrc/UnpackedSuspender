﻿#include "StdAfx.h"
#include "Memory/Memory.hpp"

using namespace std;

int main()
{
	CMemory* pMemory = new CMemory("lostsaga.exe");
	pMemory->Work();
	delete pMemory;
}