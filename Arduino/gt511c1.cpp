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

#include <arduino.h>
#include <string.h>

#include "gt511c1.h"

using namespace std;

SoftwareSerial *hComm;
char bRet;

int gnPassedTime = 0;

// firmware date
int year;
int month;
int day;

// function GetTickCount() as VisualC++
long GetTickCount(){
    return millis();
}

byte read(SoftwareSerial* vport, BYTE *buf, byte size) {
  int count = 0;
  long timeIsOut = 0;
  uint16_t timeout = 1000;
  char c;
  *buf = 0;
  timeIsOut = millis() + timeout;
  while (timeIsOut > millis() && count < size) {  
    if (vport->available()) {
      count++;
      c = vport->read();
      *buf++ = c;
      timeIsOut = millis() + timeout;
    }
  }
  if (count != 0) {
    *buf = 0;
    count++;
  }
  return count;
}

CGT511C1::CGT511C1(SoftwareSerial *vport, int dBaudrate)
{
  hComm = vport;
  hComm->begin(9600);
  
  status = true;
    
  uwDevID = DEVICE_ID;
  CommTimeOut = COMM_DEF_TIMEOUT;
  gwLastAck = 0;
  gwLastAckParam = 0;
    
  openPort();
}

CGT511C1::~CGT511C1()
{
}

int CGT511C1::openPort()
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

int CGT511C1::captureFinger(BOOL bBest)
{
  return executeCmd( CMD_CAPTUREFINGER, bBest );
}

int CGT511C1::executeCmd(WORD wCmd, int nCmdParam)
{
  if( SendCmd(uwDevID, wCmd, nCmdParam ) < 0 )
    return COMM_ERR;
  delay(700);
  if( ReceiveCmd(uwDevID, &gwLastAck, &gwLastAckParam ) < 0 )
    return COMM_ERR;
  return 0;
}

int CGT511C1::SendCmd( WORD wDevID, WORD wCmdOrAck, int nParam )
{
  CMD_PKT pkt;
  long nSentBytes;
    
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
  pkt.wChkSum0 = (BYTE)(chksum % 256);
  pkt.wChkSum1 = (BYTE)(chksum >> 8);
  
  hComm->write((BYTE*)&pkt, PKT_SIZE);
  return 0;
}

int CGT511C1::ReceiveCmd( WORD wDevID, WORD* pwCmd, int* pnParam )
{
  CMD_PKT pkt;
  long nReceivedBytes;
    
  if( (pwCmd == NULL) ||(pnParam == NULL)){
    return PKT_PARAM_ERR;
  }
    
  nReceivedBytes = read(hComm, (BYTE*)&pkt, PKT_SIZE);
  if(nReceivedBytes != PKT_SIZE)
    return PKT_COMM_ERR;
    
  if((pkt.Head1 != COMMAND_START_CODE1) ||(pkt.Head2 != COMMAND_START_CODE2))
    return PKT_HDR_ERR;
    
  WORD chksum = CalcChkSumOfCmd(&pkt);
    
  pkt.wChkSum0 = (BYTE)(chksum % 256);
  pkt.wChkSum1 = (BYTE)(chksum >> 8);
    
  if((pkt.wChkSum0 !=(BYTE)(chksum % 256))||(pkt.wChkSum1 !=(BYTE)(chksum >> 8) ))
    return PKT_CHK_SUM_ERR;
    
  *pwCmd = pkt.wCmd1<<8|pkt.wCmd0;
  *pnParam = pkt.nParam1<<8|pkt.nParam0;
    
  return 0;
}

int CGT511C1::SendData( WORD wDevID, BYTE* pBuf, int nSize )
{
  WORD wChkSum = 0;
  BYTE Buf[4], *pCommBuf;
  long nSentBytes;
  
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
  
  hComm->write(pCommBuf, nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE);
  if(pCommBuf)
    delete pCommBuf;
  return 0;
}

int CGT511C1::ReceiveData(WORD wDevID, BYTE* pBuf, int nSize, WORD timeout )
{
  WORD wReceivedChkSum, wChkSum;
  BYTE Buf[4],*pCommBuf;
  long nReceivedBytes;
    
  if( pBuf == NULL )
    return PKT_PARAM_ERR;
    
  pCommBuf = new BYTE[nSize+HEADER_SIZE+nSize+CHK_SUM_SIZE];
  nReceivedBytes = read(hComm, pCommBuf, nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE);
  if( nReceivedBytes != nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE ){
    if(pCommBuf)
      delete pCommBuf;
    return PKT_COMM_ERR;
  }
  memcpy(Buf, pCommBuf, HEADER_SIZE+DEV_ID_SIZE);
  memcpy(pBuf, pCommBuf + HEADER_SIZE + DEV_ID_SIZE, nSize);
  wReceivedChkSum = *(WORD*)(pCommBuf+nSize+HEADER_SIZE+DEV_ID_SIZE);
  if(pCommBuf)
    delete pCommBuf;
    
  if((Buf[0] != DATA_START_CODE1) || (Buf[1] != DATA_START_CODE2)){
    return PKT_HDR_ERR;
  }
    
  wChkSum = CalcChkSumOfDataPkt( Buf, HEADER_SIZE+DEV_ID_SIZE  );
  wChkSum += CalcChkSumOfDataPkt( pBuf, nSize );
    
  if(wChkSum != wReceivedChkSum)
    return PKT_CHK_SUM_ERR;
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

