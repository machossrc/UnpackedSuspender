#pragma once

#define EntryPoint 0x00401000

class CMemory
{
private:
	char m_szProcessName[MAX_PATH];
	DWORD m_dwPID;
	HANDLE m_hProcess;
private:
	DWORD GetProcessID(const char* procName);
	bool SuspendProcess(DWORD dwPID);
	bool ResumeProcess(DWORD dwPID);

	inline DWORD GetProtection(DWORD dwAddress) const;

	inline BYTE GetByte(DWORD dwAddress) const;

	inline void WaitForUnpack() const;

public:
	void Work();

public:
	void Release();


	CMemory(const char * szProcessName);
	~CMemory();
};