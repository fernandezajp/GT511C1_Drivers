#include "stdafx.h"
#include <windows.h>

#include "CCommSerial.h"

double		comm_percent = 0.0;
int			gn_comm_type = 0;

CCommSerial::CCommSerial()
{
	m_hComm = INVALID_HANDLE_VALUE;
}

CCommSerial::~CCommSerial()
{
	Close();
}

BOOL CCommSerial::Open(int nPort, DWORD dwBaudrate)
{
	Close();

	char szComName[20];
	sprintf( szComName, "COM%d", nPort );

	m_hComm = CreateFileA( szComName, GENERIC_READ | GENERIC_WRITE,0,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL );
	if(m_hComm == INVALID_HANDLE_VALUE)
		return FALSE;
	
	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	DCB dcb;
	memset(&dcb, 0, sizeof(DCB));

	dcb.DCBlength = sizeof(DCB) ;
	
	dcb.BaudRate = dwBaudrate;
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;

	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	dcb.fAbortOnError = TRUE;
	
	if(!SetCommState( m_hComm, &dcb ))
		return FALSE;

	COMMTIMEOUTS to;
	to.ReadIntervalTimeout = 0;
	to.ReadTotalTimeoutMultiplier = 0;
	to.ReadTotalTimeoutConstant = 16;
	to.WriteTotalTimeoutMultiplier = 0;
	to.WriteTotalTimeoutConstant = 16;

	if(!SetCommTimeouts(m_hComm, &to))
		return FALSE;

	return TRUE;
}

void CCommSerial::Close()
{
	if ( m_hComm != INVALID_HANDLE_VALUE )
	{
		CloseHandle(m_hComm) ;
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

int CCommSerial::SendData(BYTE *pBuf, int nSize, DWORD dwTimeOut)
{
	int nReqSize = nSize;
	int nWrittenSize = 0;
	
	DWORD dwStartTime = GetTickCount();
	while (nSize && GetTickCount() - dwStartTime < dwTimeOut)
	{
		DWORD nCurrentWrittenSize;
		BOOL bWrite = WriteFile(m_hComm, pBuf, nSize, &nCurrentWrittenSize, NULL);
				
		if (!bWrite)
			break;
		FlushFileBuffers( m_hComm );
		pBuf += nCurrentWrittenSize;
		nSize -= nCurrentWrittenSize;
		nWrittenSize += nCurrentWrittenSize;
		
		if (nCurrentWrittenSize)
			dwStartTime = GetTickCount();

		comm_percent = (double)nWrittenSize / (double)nReqSize * 100.0;
	}

	return nWrittenSize;
}


int CCommSerial::RecvData(BYTE *pBuf, int nSize, DWORD dwTimeOut)
{
	int nReqSize = nSize;
	int nReadSize = 0;

	DWORD dwStartTime = GetTickCount();
	while (nSize && GetTickCount() - dwStartTime < dwTimeOut)
	{
		DWORD nCurrentReadSize;
		BOOL bRead = ReadFile(m_hComm, pBuf, nSize, &nCurrentReadSize, NULL);

		if (!bRead)
			break;
		pBuf += nCurrentReadSize;
		nSize -= nCurrentReadSize;
		nReadSize += nCurrentReadSize;

		if (nCurrentReadSize)
			dwStartTime = GetTickCount();
	
		comm_percent = (double)nReadSize / (double)nReqSize * 100.0;
	}

	return nReadSize;
}
