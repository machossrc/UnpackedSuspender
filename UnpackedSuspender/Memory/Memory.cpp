#include "../StdAfx.h"
#include "Memory.hpp"
#include <TlHelp32.h>

using namespace std;

CMemory::CMemory(const char * szProcessName)
{
	Release();
	strcpy(m_szProcessName, szProcessName);
}

CMemory::~CMemory()
{
	Release();
}

void CMemory::Release()
{
	m_dwEntryPoint = 0;
	m_hProcess = NULL;
	m_dwPID = 0;
	ZeroMemory(m_szProcessName, MAX_PATH);
}

DWORD CMemory::GetProcessID(const char* procName)
{
	DWORD procID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);
		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_stricmp(procEntry.szExeFile, procName))
				{
					procID = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procID;
}

DWORD CMemory::GetModuleBaseAddress(const char* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_dwPID);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_stricmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

bool CMemory::SuspendProcess()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	if (Thread32First(hSnapShot, &te))
	{
		do
		{
			if (te.th32OwnerProcessID == m_dwPID)
			{
				if (te.th32ThreadID == GetCurrentThreadId()) //자긴 제외
				{
					continue;
				}

				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
				if (hThread)
				{
					SuspendThread(hThread);
					CloseHandle(hThread);
				}
			}
		} while (Thread32Next(hSnapShot, &te));
	}
	else
	{
		return false;
	}

	CloseHandle(hSnapShot);
}

bool CMemory::ResumeProcess()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	if (Thread32First(hSnapShot, &te))
	{
		do
		{
			if (te.th32OwnerProcessID == m_dwPID)
			{
				if (te.th32ThreadID == GetCurrentThreadId()) //자긴 제외
				{
					continue;
				}

				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
				if (hThread)
				{
					ResumeThread(hThread);
					CloseHandle(hThread);
				}
			}
		} while (Thread32Next(hSnapShot, &te));
	}
	else
	{
		return false;
	}

	CloseHandle(hSnapShot);
}

inline DWORD CMemory::GetProtection(DWORD dwAddress) const
{
	DWORD dwProtect = 0;

	MEMORY_BASIC_INFORMATION mbi;

	if (VirtualQueryEx(m_hProcess, (LPVOID)dwAddress, &mbi, sizeof(mbi)) != 0)
	{
		dwProtect = mbi.Protect;
	}

	return dwProtect;
}

inline BYTE CMemory::GetByte(DWORD dwAddress) const
{
	BYTE byData = 0;

	ReadProcessMemory(m_hProcess, (BYTE*)dwAddress, &byData, sizeof(byData), NULL);

	return byData;
}

inline void CMemory::WaitForUnpack() const
{
	while (true)
	{
		if (GetByte(m_dwEntryPoint) == (BYTE)0x55 && GetProtection(m_dwEntryPoint) == 0x20)
		{
			break;
		}
	}
}



void CMemory::Work()
{
	while (true)
	{
		m_dwPID = GetProcessID(m_szProcessName);
		if (m_dwPID != 0)
		{
			break;
		}
	}

	m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_dwPID);

	while (true)
	{
		m_dwEntryPoint = GetModuleBaseAddress(m_szProcessName) + 0x1000;
		if (m_dwEntryPoint > 0x1000)
		{
			break;
		}
	}

	WaitForUnpack();

	SuspendProcess();

	printf("Suspend!!\nEntryPoint : %0x\n", m_dwEntryPoint);

	MEMORY_BASIC_INFORMATION mbi;
	VirtualQueryEx(m_hProcess, (LPVOID)m_dwEntryPoint, &mbi, sizeof(mbi));

	DWORD dwProtect = 0;
	VirtualProtectEx(m_hProcess, (LPVOID)m_dwEntryPoint, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwProtect);


	system("pause");

	VirtualProtectEx(m_hProcess, (LPVOID)m_dwEntryPoint, mbi.RegionSize, dwProtect, &dwProtect);

	ResumeProcess();

}