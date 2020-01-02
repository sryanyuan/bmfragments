#include "ShareMem.h"
#include <Windows.h>

ShareMem::ShareMem(const char *name) {
	m_hFileHandle = NULL;
	m_szName = name;
	m_nLastErr = 0;
	m_nLen = 0;
	m_bWrite = false;
}

ShareMem::~ShareMem() {
	Close();
}

int ShareMem::GetLastErr() {
	return m_nLastErr;
}

void ShareMem::Close() {
	CloseHandle(m_hFileHandle);
	m_hFileHandle = NULL;
}

bool ShareMem::Open(bool w, int len) {
	if (NULL != m_hFileHandle) {
		return false;
	}

	if (!w) {
		m_hFileHandle = OpenFileMapping(FILE_MAP_READ, FALSE, m_szName);
	}
	else {
		m_hFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, len, m_szName);
	}

	if (NULL == m_hFileHandle) {
		m_nLastErr = GetLastError();
		return false;
	}

	m_nLen = len;
	m_nLastErr = 0;
	return true;
}

bool ShareMem::Read(char* buf, int len) {
	if (NULL == m_hFileHandle) {
		return false;
	}

	const char* rbuf = (const char*)MapViewOfFile(m_hFileHandle, FILE_MAP_READ, 0, 0, len);
	if (nullptr == rbuf) {
		m_nLastErr = GetLastError();
		return false;
	}

	memcpy(buf, rbuf, len);
	return true;
}

bool ShareMem::Write(const char* buf, int len) {
	if (NULL == m_hFileHandle) {
		return false;
	}

	char* wbuf = (char*)MapViewOfFile(m_hFileHandle, FILE_MAP_ALL_ACCESS, 0, 0, len);
	if (nullptr == wbuf) {
		m_nLastErr = GetLastError();
		return false;
	}

	memcpy(wbuf, buf, len);
	return true;
}
