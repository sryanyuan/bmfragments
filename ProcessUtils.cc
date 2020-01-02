#include "ProcessUtils.h"
#include <Windows.h>

bool CreateChildProcess(const char* szChildPath, const char *name) {
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	HANDLE hd = CreateJobObject(nullptr, name);
	if (NULL == hd) {
		return false;
	}

	BOOL bRet = CreateProcess(szChildPath, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
	if (!bRet) {
		CloseHandle(hd);
		return false;
	}

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION li;
	li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	bRet = SetInformationJobObject(hd, JobObjectExtendedLimitInformation, &li, sizeof(li));
	if (bRet) {
		bRet = AssignProcessToJobObject(hd, pi.hProcess);
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return bRet == TRUE;
}
