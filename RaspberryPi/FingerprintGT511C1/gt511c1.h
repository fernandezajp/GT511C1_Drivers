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

#pragma once

#define CMD_OPEN					0x01
#define CMD_CLOSE					0x02
#define CMD_USBINTERNALCHECK		0x03
#define CMD_CHANGEBAUDRATE			0x04
#define CMD_SETIAPMODE				0x05
#define CMD_CMOSLED					0x12
#define CMD_GETENROLLCOUNT			0x20
#define CMD_CHECKENROLLED			0x21
#define CMD_ENROLLSTART				0x22
#define CMD_ENROLL1					0x23
#define CMD_ENROLL2					0x24
#define CMD_ENROLL3					0x25
#define CMD_ISPRESSFINGER			0x26
#define CMD_DELETEID				0x40
#define CMD_DELETEALL				0x41
#define CMD_VERIFY					0x50
#define CMD_IDENTIFY				0x51
#define CMD_VERIFYTEMPLATE			0x52
#define CMD_IDENTIFYTEMPLATE		0x53
#define CMD_CAPTUREFINGER			0x60
#define CMD_MAKETEMPLATE			0x61
#define CMD_GETIMAGE				0x62
#define CMD_GETRAWIMAGE				0x63
#define CMD_GETTEMPLATE				0x70
#define CMD_SETTEMPLATE				0x71
#define CMD_GETDATABASESTART		0x72
#define CMD_GETDATABASEEND			0x73
#define CMD_UPGRADEFIRMWARE			0x80
#define CMD_UPGRADEISOCDIMAGE		0x81
#define CMD_ACK						0x30
#define CMD_NACK					0x31

#define NACK_IDENTIFY_FAILED		0x1008
#define NACK_DB_IS_EMPTY			0x100A

#define OK							0
#define DEVICE_ID					0x01
#define COMMAND_START_CODE1			0x55
#define COMMAND_START_CODE2			0xAA
#define DATA_START_CODE1			0x5A
#define DATA_START_CODE2			0xA5
#define HEADER_SIZE					2
#define DEV_ID_SIZE					2

#define RESP_PKT					CMD_PKT
#define PKT_SIZE					sizeof(CMD_PKT)
#define CHK_SUM_SIZE				2
#define COMM_DEF_TIMEOUT			3000
#define FP_TEMPLATE_SIZE			506

#define PKT_ERR_START				-500
#define PKT_COMM_ERR				PKT_ERR_START+1
#define PKT_HDR_ERR					PKT_ERR_START+2
#define PKT_DEV_ID_ERR				PKT_ERR_START+3
#define PKT_CHK_SUM_ERR				PKT_ERR_START+4
#define PKT_PARAM_ERR				PKT_ERR_START+5

#define IMG8BIT_SIZE	256*256

typedef bool          BOOL;
typedef unsigned char BYTE;
typedef unsigned int  WORD;

enum { NONE = -2000, COMM_ERR, };

typedef struct {
	unsigned char 	Head1;
	unsigned char 	Head2;
	unsigned char	wDevId0;
	unsigned char	wDevId1;
	unsigned char	nParam0;
	unsigned char	nParam1;
	unsigned char	nParam2;
	unsigned char	nParam3;
	unsigned char	wCmd0;
	unsigned char	wCmd1;
	unsigned char 	wChkSum0;
	unsigned char 	wChkSum1;
} CMD_PKT;

class CGT511C1
{
public:
    CGT511C1();
	CGT511C1(const char*, int);
	virtual ~CGT511C1();

	int openPort(void);
	int cmosLed(BOOL);
	int getEnrollCount(void);
	int checkEnrolled(int);
	int enroll_start(int);
	int enroll_num(int);
	int isPressFinger(void);
	int deleteId(int);
	int deleteAll(void);
	int verify(int);
	int identify(void);
	int verifyTemplate(int);
	int identifyTemplate(void);
	int captureFinger(BOOL);
	int makeTemplate(void);
	int getImage(void);
	int getRawImage(void);
	int getTemplate(int);

	WORD gwLastAck;
	int  gwLastAckParam;
	BYTE byTemplate[FP_TEMPLATE_SIZE];
	bool status;

private:
	WORD uwDevID;
	WORD CommTimeOut;
	int firmwareYear;
	int firmwareMonth;
	int firmwareDay;

	int executeCmd(WORD, int);
	int SendCmd(WORD, WORD, int);
	int ReceiveCmd(WORD, WORD*, int*);
	int SendData(WORD, BYTE* , int);
	int ReceiveData(WORD, BYTE*, int, WORD);
	WORD CalcChkSumOfCmd(CMD_PKT*);
	WORD CalcChkSumOfDataPkt( BYTE*, int );
};
