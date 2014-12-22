#pragma once

class CCommSerial
{
public:
	CCommSerial();
	virtual ~CCommSerial();
	
	BOOL Open(int nPort, DWORD dwBaudrate);
	void Close();
	
	int SendData(BYTE *pBuf, int nSize, DWORD dwTimeOut);
	int RecvData(BYTE *pBuf, int nSize, DWORD dwTimeOut);
	
private:
	HANDLE m_hComm;
};