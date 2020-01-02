#ifndef _INC_SHAREMEM_
#define _INC_SHAREMEM_

#include <Windows.h>

class ShareMem {
public:
	ShareMem(const char* name);
	~ShareMem();

public:
	int GetLastErr();
	bool Open(bool w, int len);
	void Close();

	bool Read(char* buf, int len);
	bool Write(const char* buf, int len);

private:
	bool m_bWrite;
	int m_nLen;
	int m_nLastErr;
	HANDLE m_hFileHandle;
	const char* m_szName;
};

#endif
