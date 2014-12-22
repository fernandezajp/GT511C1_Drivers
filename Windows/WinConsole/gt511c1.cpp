// This software is based on the source code of SDK Fingerprint Demo of Beijing Smackbio Technology Co., Ltd
// and is published under the GPL license because there is no prior indication of this.
// Copyright (C) Alvaro Fernandez

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>

#include "CCommSerial.h"
#include "gt511c1.h"

HANDLE hComm;
char bRet;
unsigned char responseBuffer[12];
unsigned char datapacketBuffer[2048];

BYTE	gbyImg8bit[IMG8BIT_SIZE];
BYTE	gbyImgRaw[320*240];
BYTE	gbyTemplate[FP_TEMPLATE_SIZE];
int		gnPassedTime = 0;

static	BYTE	gbyImg256_2[216*240];
static	BYTE	gbyImg256_tmp[240*216];
static  BYTE	gbyImgRaw2[240*320/4];


// firmware date
int year;
int month;
int day;

// delay as Arduino
void delay(int millisecs) {
	Sleep(millisecs);
}

CGT511C1::CGT511C1()
{}

CGT511C1::CGT511C1(int nPort, DWORD dwBaudrate)
{
	status = false;
	status = commport.Open(nPort, dwBaudrate);

	uwDevID = DEVICE_ID;
	CommTimeOut = COMM_DEF_TIMEOUT;
	gwLastAck = 0;
	gwLastAckParam = 0;

	open();
}

CGT511C1::~CGT511C1()
{
	commport.Close();
}

int CGT511C1::open()
{
	if (executeCmd( CMD_OPEN, 1 ) < 0 )
		return COMM_ERR;

	delay(100);

	BYTE Data[6];

	if (ReceiveData( uwDevID, (BYTE*)&Data, 6, 100) < 0 )
		return COMM_ERR;

	firmwareYear = Data[0]*100+Data[1];
	firmwareMonth = Data[2];
	firmwareDay = Data[3];

	return 0;
}

int CGT511C1::cmosLed(BOOL bOn)
{
	return executeCmd( CMD_CMOSLED, bOn ? 1 : 0 );
}

int CGT511C1::getEnrollCount()
{
	return executeCmd( CMD_GETENROLLCOUNT, 0 );
}

int CGT511C1::checkEnrolled(int nId)
{
	return executeCmd( CMD_CHECKENROLLED, nId );
}

int CGT511C1::enroll_start(int nId)
{
	return executeCmd( CMD_ENROLLSTART, nId );
}

int CGT511C1::enroll_num(int nTurn)
{
	return executeCmd( CMD_ENROLLSTART+nTurn, 0 );
}

int CGT511C1::isPressFinger()
{
	return executeCmd( CMD_ISPRESSFINGER, 0 );
}

int CGT511C1::deleteId(int nId)
{
	return executeCmd( CMD_DELETEID, nId );
}

int CGT511C1::deleteAll()
{
	return executeCmd( CMD_DELETEALL, 0 );
}

int CGT511C1::verify(int nId)
{
	return executeCmd(CMD_VERIFY, nId);
}

int CGT511C1::identify()
{
	return executeCmd( CMD_IDENTIFY, 0 );
}

int CGT511C1::verifyTemplate(int nId)
{
	if( executeCmd( CMD_VERIFYTEMPLATE, nId ) < 0 )
		return COMM_ERR;
	
	if(gwLastAck == CMD_ACK)
	{
		if( SendData( uwDevID, &gbyTemplate[0], FP_TEMPLATE_SIZE ) < 0 )
			return COMM_ERR;
		
		gnPassedTime = GetTickCount();
		if( ReceiveCmd( uwDevID, &gwLastAck, &gwLastAckParam ) < 0 )
			return COMM_ERR;
		gnPassedTime = GetTickCount() - gnPassedTime;
	}
	
	return 0;
}

int CGT511C1::identifyTemplate()
{
	if( executeCmd( CMD_IDENTIFYTEMPLATE, 0 ) < 0 )
		return COMM_ERR;
	
	if(gwLastAck == CMD_ACK)
	{
		if( SendData(uwDevID, &gbyTemplate[0], FP_TEMPLATE_SIZE ) < 0 )
			return COMM_ERR;
		
		gnPassedTime = GetTickCount();
		if( ReceiveCmd(uwDevID, &gwLastAck, &gwLastAckParam ) < 0 )
			return COMM_ERR;
		gnPassedTime = GetTickCount() - gnPassedTime;
	}
	
	return 0;
}

int CGT511C1::captureFinger(BOOL bBest)
{
	return executeCmd( CMD_CAPTUREFINGER, bBest );
}

int CGT511C1::makeTemplate()
{
	if( executeCmd(CMD_MAKETEMPLATE, 0) < 0 )
		return COMM_ERR;

	if(gwLastAck == CMD_ACK)
	{
		if( ReceiveData( uwDevID, &byTemplate[0], FP_TEMPLATE_SIZE, 3000 ) < 0 )
			return COMM_ERR;
	}
	
	return 0;
}

int CGT511C1::getImage()
{
	if( executeCmd( CMD_GETIMAGE, 0 ) < 0 )
		return COMM_ERR;
	
	if( ReceiveData( uwDevID, gbyImg256_tmp, sizeof gbyImg256_tmp, 3000 ) < 0 )
		return COMM_ERR;
	
	// image rotate
	int i, j;

	for( i=0; i<216; i++)
	{
		for( j=0; j<240; j++)
		{
			gbyImg256_2[i*240+j] = gbyImg256_tmp[j*216+i];
		}
	}
	
	memset(gbyImg8bit, 161, sizeof(gbyImg8bit));
	
	for (i=0; i<202; i++)
	{
		memcpy(&gbyImg8bit[256*(27 + i) + 0], &gbyImg256_2[i * 258 + 1], 256);
	}
	
	return 0;
}

int CGT511C1::getRawImage()
{
	if( executeCmd( CMD_GETRAWIMAGE, 0 ) < 0 )
		return COMM_ERR;
	
	if( ReceiveData( uwDevID, gbyImgRaw2, sizeof gbyImgRaw2, 3000 ) < 0 )
		return COMM_ERR;

	/*AVW*/
	memset(gbyImgRaw, 66, sizeof gbyImgRaw);
	int i, j;
	for (i=0; i<120; i++)
	{
		for(j=0; j< 160; j++)
		{
			gbyImgRaw[320*(2*i+0)+(2*j+0)] = gbyImgRaw2[i*160+j];
			gbyImgRaw[320*(2*i+0)+(2*j+1)] = gbyImgRaw2[i*160+j];
			gbyImgRaw[320*(2*i+1)+(2*j+0)] = gbyImgRaw2[i*160+j];
			gbyImgRaw[320*(2*i+1)+(2*j+1)] = gbyImgRaw2[i*160+j];
		}		
	}

	return 0;
}

int CGT511C1::getTemplate(int nPos)
{
	if( executeCmd(CMD_GETTEMPLATE, nPos) < 0 )
		return COMM_ERR;

	if(gwLastAck == CMD_ACK)
	{
		if( ReceiveData( uwDevID, &byTemplate[0], FP_TEMPLATE_SIZE, 3000 ) < 0 )
			return COMM_ERR;
	}
	
	return 0;
}


int CGT511C1::executeCmd(WORD wCmd, int nCmdParam)
{
	if( SendCmd(uwDevID, wCmd, nCmdParam ) < 0 )
		return COMM_ERR;
	delay(100);
	if( ReceiveCmd(uwDevID, &gwLastAck, &gwLastAckParam ) < 0 )
		return COMM_ERR;
	return 0;
}

int CGT511C1::SendCmd( WORD wDevID, WORD wCmdOrAck, int nParam )
{
	CMD_PKT pkt;
	int nSentBytes;

	pkt.Head1 = (BYTE)COMMAND_START_CODE1;
	pkt.Head2 = (BYTE)COMMAND_START_CODE2;
	pkt.wDevId0 = wDevID;
	pkt.wDevId1 = wDevID>>8;
	pkt.nParam0 = nParam & 0xFF;
	pkt.nParam1 = (nParam & 0xFF00) >> 8 ;
	pkt.nParam2 = (nParam & 0xFF0000) >> 16;
	pkt.nParam3 = (nParam & 0xFF000000) >> 24;
	pkt.wCmd0 = wCmdOrAck;
	pkt.wCmd1 = 0x00;
	WORD chksum = CalcChkSumOfCmd( &pkt );
	pkt.wChkSum0 = (byte)(chksum % 256);
	pkt.wChkSum1 = (byte)(chksum >> 8);

	nSentBytes = commport.SendData((BYTE*)&pkt, PKT_SIZE, CommTimeOut);
	if( nSentBytes != PKT_SIZE )
		return PKT_COMM_ERR;

	return 0;
}

int CGT511C1::ReceiveCmd( WORD wDevID, WORD* pwCmd, int* pnParam )
{
	CMD_PKT pkt;
	int nReceivedBytes;
		
	if( ( pwCmd == NULL ) || 
		( pnParam == NULL ) )
	{
		return PKT_PARAM_ERR;
	}

	nReceivedBytes = commport.RecvData((BYTE*)&pkt, PKT_SIZE, CommTimeOut);
	if( nReceivedBytes != PKT_SIZE )
		return PKT_COMM_ERR;

	if( ( pkt.Head1 != COMMAND_START_CODE1 ) || 
		( pkt.Head2 != COMMAND_START_CODE2 ) )
		return PKT_HDR_ERR;

	WORD chksum = CalcChkSumOfCmd(&pkt);

	pkt.wChkSum0 = (byte)(chksum % 256);
	pkt.wChkSum1 = (byte)(chksum >> 8);

	if( (pkt.wChkSum0 !=(byte)(chksum % 256)) ||
		(pkt.wChkSum1 !=(byte)(chksum >> 8) )) 
		return PKT_CHK_SUM_ERR;
	
	*pwCmd = pkt.wCmd1<<8|pkt.wCmd0;
	*pnParam = pkt.nParam1<<8|pkt.nParam0;
	
	return 0;
}

int CGT511C1::SendData( WORD wDevID, BYTE* pBuf, int nSize )
{
	WORD wChkSum = 0;
	BYTE Buf[4], *pCommBuf;
	int nSentBytes;
	
	if( pBuf == NULL )
		return PKT_PARAM_ERR;

	Buf[0] = (BYTE)DATA_START_CODE1;
	Buf[1] = (BYTE)DATA_START_CODE2;
	*((WORD*)(&Buf[HEADER_SIZE])) = wDevID;
	
	wChkSum = CalcChkSumOfDataPkt( Buf, HEADER_SIZE+DEV_ID_SIZE  );
	wChkSum += CalcChkSumOfDataPkt( pBuf, nSize );
	
	pCommBuf = new BYTE[nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE];
	memcpy(pCommBuf, Buf, HEADER_SIZE+DEV_ID_SIZE);
	memcpy(pCommBuf+HEADER_SIZE+DEV_ID_SIZE, pBuf, nSize);
	*(WORD*)(pCommBuf+nSize+HEADER_SIZE+DEV_ID_SIZE) = wChkSum;

	nSentBytes = commport.SendData( pCommBuf, nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE, CommTimeOut );
	if( nSentBytes != nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE )
	{
		if(pCommBuf)
			delete pCommBuf;
		return PKT_COMM_ERR;
	}

	if(pCommBuf)
		delete pCommBuf;

	return 0;
}

int CGT511C1::ReceiveData(WORD wDevID, BYTE* pBuf, int nSize, DWORD timeout )
{
	WORD wReceivedChkSum, wChkSum;
	BYTE Buf[4],*pCommBuf;
	int nReceivedBytes;
	
	if( pBuf == NULL )
		return PKT_PARAM_ERR;

	pCommBuf = new BYTE[nSize+HEADER_SIZE+nSize+CHK_SUM_SIZE];
	nReceivedBytes = commport.RecvData( pCommBuf, nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE, timeout );
	if( nReceivedBytes != nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE )
	{
		if(pCommBuf)
			delete pCommBuf;
		return PKT_COMM_ERR;
	}
	memcpy(Buf, pCommBuf, HEADER_SIZE+DEV_ID_SIZE);
	memcpy(pBuf, pCommBuf + HEADER_SIZE + DEV_ID_SIZE, nSize);
	wReceivedChkSum = *(WORD*)(pCommBuf+nSize+HEADER_SIZE+DEV_ID_SIZE);
	if(pCommBuf)
		delete pCommBuf;
	////////////// pc end ///////////////

	if( ( Buf[0] != DATA_START_CODE1 ) || 
		( Buf[1] != DATA_START_CODE2 ) )
	{
		return PKT_HDR_ERR;
	}
	
	/*if( *((WORD*)(&Buf[SB_OEM_HEADER_SIZE])) != wDevID ) 
		return PKT_DEV_ID_ERR;*/
	
	wChkSum = CalcChkSumOfDataPkt( Buf, HEADER_SIZE+DEV_ID_SIZE  );
	wChkSum += CalcChkSumOfDataPkt( pBuf, nSize );
	
	if( wChkSum != wReceivedChkSum ) 
		return PKT_CHK_SUM_ERR;
	/*AVW modify*/
	return 0;
}

WORD CGT511C1::CalcChkSumOfCmd( CMD_PKT* pPkt )
{
	WORD wChkSum = 0;
	BYTE* pBuf = (BYTE*)pPkt;
	int i;
	
	for(i=0;i<(PKT_SIZE-CHK_SUM_SIZE);i++)
		wChkSum += pBuf[i];
	return wChkSum;
}

WORD CGT511C1::CalcChkSumOfDataPkt( BYTE* pDataPkt, int nSize )
{
	int i;
	WORD wChkSum = 0;
	BYTE* pBuf = (BYTE*)pDataPkt;
	
	for(i=0;i<nSize;i++)
		wChkSum += pBuf[i];
	return wChkSum;
}

