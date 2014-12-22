using System;
using System.IO.Ports;
using Microsoft.SPOT;
using System.Threading;

namespace program
{  
    using RESP_PKT = CMD_PKT;

    using BYTE = Byte;
    using WORD = UInt16;
    using DWORD = UInt32;
    
    class CGT511C1
    {
        const byte GET_DATA = 0xAE;
        const byte CMD_OPEN	= 0x01;
        const byte CMD_CLOSE = 0x02;
        const byte CMD_USBINTERNALCHECK	= 0x03;
        const byte CMD_CHANGEBAUDRATE = 0x04;
        const byte CMD_SETIAPMODE = 0x05;
        const byte CMD_CMOSLED = 0x12;
        const byte CMD_GETENROLLCOUNT = 0x20;
        const byte CMD_CHECKENROLLED = 0x21;
        const byte CMD_ENROLLSTART = 0x22;
        const byte CMD_ENROLL1 = 0x23;
        const byte CMD_ENROLL2 = 0x24;
        const byte CMD_ENROLL3 = 0x25;
        const byte CMD_ISPRESSFINGER = 0x26;
        const byte CMD_DELETEID = 0x40;
        const byte CMD_DELETEALL = 0x41;
        const byte CMD_VERIFY = 0x50;
        const byte CMD_IDENTIFY = 0x51;
        const byte CMD_VERIFYTEMPLATE = 0x52;
        const byte CMD_IDENTIFYTEMPLATE = 0x53;
        const byte CMD_CAPTUREFINGER = 0x60;
        const byte CMD_MAKETEMPLATE = 0x61;
        const byte CMD_GETIMAGE = 0x62;
        const byte CMD_GETRAWIMAGE = 0x63;
        const byte CMD_GETTEMPLATE = 0x70;
        const byte CMD_SETTEMPLATE = 0x71;
        const byte CMD_GETDATABASESTART = 0x72;
        const byte CMD_GETDATABASEEND = 0x73;
        const byte CMD_UPGRADEFIRMWARE = 0x80;
        const byte CMD_UPGRADEISOCDIMAGE = 0x81;
        const byte CMD_ACK = 0x30;
        const byte CMD_NACK = 0x31;

        const Int16 NACK_IDENTIFY_FAILED = 0x1008;
        const Int16 NACK_DB_IS_EMPTY = 0x100A;

        const byte OK = 0;
        const byte DEVICE_ID = 0x01;
        const byte COMMAND_START_CODE1 = 0x55;
        const byte COMMAND_START_CODE2 = 0xAA;
        const byte DATA_START_CODE1 = 0x5A;
        const byte DATA_START_CODE2 = 0xA5;
        const byte HEADER_SIZE = 2;
        const byte DEV_ID_SIZE = 2;

        const byte PKT_SIZE = 12; //sizeof(CMD_PKT) = 12
        const byte CHK_SUM_SIZE	= 2;
        const WORD COMM_DEF_TIMEOUT = 3000;
        const WORD FP_TEMPLATE_SIZE = 506;

        const Int16 PKT_ERR_START = -500;
        const Int16 PKT_COMM_ERR = PKT_ERR_START + 1;
        const Int16 PKT_HDR_ERR = PKT_ERR_START + 2;
        const Int16 PKT_DEV_ID_ERR = PKT_ERR_START + 3;
        const Int16 PKT_CHK_SUM_ERR = PKT_ERR_START + 4;
        const Int16 PKT_PARAM_ERR = PKT_ERR_START + 5;

        const int COMM_ERR = -1;

        public bool status;
        public WORD gwLastAck;
        public int gwLastAckParam;

        SerialPort serial;
        WORD uwDevID;
        DWORD CommTimeOut;

        int firmwareYear;
        int firmwareMonth;
        int firmwareDay;

        public CGT511C1(int nPort, int dwBaudrate)
        {
            //serial = new SerialPort ( "COM"+nPort.ToString() , 9600, Parity.None, 8, StopBits.One);
            //uwDevID = DEVICE_ID;
            //CommTimeOut = COMM_DEF_TIMEOUT;
            //gwLastAck = 0;
            //gwLastAckParam = 0;
            //serial.ReadTimeout = (int)CommTimeOut;

            serial = new SerialPort("COM" + nPort.ToString(), 9600, Parity.None, 8, StopBits.One);
            uwDevID = DEVICE_ID;
            CommTimeOut = COMM_DEF_TIMEOUT;
            gwLastAck = 0;
            gwLastAckParam = 0;
            serial.ReadTimeout = (int)CommTimeOut;
            serial.Open();
            change_baudrate(dwBaudrate);
            serial.Close();
            serial = new SerialPort("COM" + nPort.ToString(), dwBaudrate, Parity.None, 8, StopBits.One);
            serial.ReadTimeout = (int)CommTimeOut;

            open();
        }

        // nBaudrate = 9600, 19200, 38400, 57600, 115200
        int change_baudrate(int nBaudrate) {
	        return executeCmd( CMD_CHANGEBAUDRATE, nBaudrate );
        }

        public int cmosLed(bool bOn)
        {
	        return executeCmd( CMD_CMOSLED, bOn ? 1 : 0 );
        }

        public int getEnrollCount()
        {
	        return executeCmd( CMD_GETENROLLCOUNT, 0 );
        }

        public int checkEnrolled(int nId)
        {
	        return executeCmd( CMD_CHECKENROLLED, nId );
        }

        public int enroll_start(int nId)
        {
	        return executeCmd( CMD_ENROLLSTART, nId );
        }

        public int enroll_num(int nTurn)
        {
	        return executeCmd( (BYTE)(CMD_ENROLLSTART+nTurn), 0 );
        }

        public int isPressFinger()
        {
	        return executeCmd( CMD_ISPRESSFINGER, 0 );
        }

        public int deleteId(int nId)
        {
	        return executeCmd( CMD_DELETEID, nId );
        }

        public int deleteAll()
        {
	        return executeCmd( CMD_DELETEALL, 0 );
        }

        public int verify(int nId)
        {
	        return executeCmd(CMD_VERIFY, nId);
        }

        public int identify()
        {
	        return executeCmd( CMD_IDENTIFY, 0 );
        }

        public int captureFinger(bool bBest)
        {
	        return executeCmd( CMD_CAPTUREFINGER, bBest?1:0);
        }

        int open()
        {
            serial.Open();
            status = true;

	        if (executeCmd( CMD_OPEN, 1 ) < 0 )
		        return COMM_ERR;

	        Thread.Sleep(100);

            BYTE[] Data = new BYTE[6];

	        if (ReceiveData( uwDevID, ref Data, 6, 100) < 0 )
		        return COMM_ERR;

	        firmwareYear = Data[0]*100+Data[1];
	        firmwareMonth = Data[2];
	        firmwareDay = Data[3];

	        return 0;
        }

        int executeCmd(BYTE wCmd, int nCmdParam)
        {
            SendCmd(uwDevID, wCmd, nCmdParam);
            Thread.Sleep(100);
	        if( ReceiveCmd(uwDevID, ref gwLastAck, ref gwLastAckParam ) < 0 )
		        return COMM_ERR;
	        return 0;
        }

        void SendCmd( WORD wDevID, BYTE wCmdOrAck, int nParam )
        {
	        CMD_PKT pkt = new CMD_PKT();
            BYTE[] pktBuffer = new BYTE[12];

	        pkt.Head1 = (BYTE)COMMAND_START_CODE1;
	        pkt.Head2 = (BYTE)COMMAND_START_CODE2;
	        pkt.wDevId0 = (byte)(wDevID & 0xFF);
	        pkt.wDevId1 = (byte)(wDevID>>8);
	        pkt.nParam0 = (byte)(nParam & 0xFF);
	        pkt.nParam1 = (byte)((nParam & 0xFF00) >> 8) ;
	        pkt.nParam2 = (byte)((nParam & 0xFF0000) >> 16);
	        pkt.nParam3 = (byte)((nParam & 0xFF000000) >> 24);
	        pkt.wCmd0 = wCmdOrAck;
	        pkt.wCmd1 = 0x00;
	        WORD chksum = CalcChkSumOfCmd( ref pkt );
	        pkt.wChkSum0 = (byte)(chksum % 256);
	        pkt.wChkSum1 = (byte)(chksum >> 8);

            pktBuffer = PktToByteArray(ref pkt);

            serial.Write(pktBuffer, 0, PKT_SIZE);
        }

        int ReceiveCmd( WORD wDevID, ref WORD wCmd, ref int nParam )
        {
	        CMD_PKT pkt;
            BYTE[] CommBuf;
            int nReceivedBytes;

            CommBuf = new BYTE[12];
            nReceivedBytes = serial.Read(CommBuf, 0, PKT_SIZE);
	        if( nReceivedBytes != PKT_SIZE )
		        return PKT_COMM_ERR;

            if ((CommBuf[0] != COMMAND_START_CODE1) || (CommBuf[1] != COMMAND_START_CODE2))
		        return PKT_HDR_ERR;

            pkt = ByteArrayToPkt(ref CommBuf);

	        WORD chksum = CalcChkSumOfCmd(ref pkt);

	        pkt.wChkSum0 = (byte)(chksum % 256);
	        pkt.wChkSum1 = (byte)(chksum >> 8);

	        if( (pkt.wChkSum0 !=(byte)(chksum % 256)) ||
		        (pkt.wChkSum1 !=(byte)(chksum >> 8) )) 
		        return PKT_CHK_SUM_ERR;
	
	        wCmd = (WORD)(pkt.wCmd1<<8 | pkt.wCmd0);
	        nParam = pkt.nParam1<<8 | pkt.nParam0;
	
	        return 0;
        }

        int ReceiveData(WORD wDevID, ref BYTE[] pBuf, int nSize, DWORD timeout )
        {
	        WORD wReceivedChkSum, wChkSum;
            BYTE[] CommBuf;
	        int nReceivedBytes;

	        if(pBuf == null)
		        return PKT_PARAM_ERR;

            CommBuf = new BYTE[nSize + HEADER_SIZE + DEV_ID_SIZE + CHK_SUM_SIZE];
	        nReceivedBytes = serial.Read(CommBuf,0,nSize+HEADER_SIZE+DEV_ID_SIZE+CHK_SUM_SIZE);
	        if( nReceivedBytes != nSize + HEADER_SIZE + DEV_ID_SIZE + CHK_SUM_SIZE )
		        return PKT_COMM_ERR;
            
            wReceivedChkSum = (WORD)((WORD)(CommBuf[nSize+HEADER_SIZE+DEV_ID_SIZE+1]<<0x08)|(WORD)(CommBuf[nSize+HEADER_SIZE+DEV_ID_SIZE])); 

	        if((CommBuf[0] != DATA_START_CODE1) || (CommBuf[1] != DATA_START_CODE2))
		        return PKT_HDR_ERR;

            for (int i = 0; i < nSize; i++)
                pBuf[i] = CommBuf[i + HEADER_SIZE + DEV_ID_SIZE];

            wChkSum = CalcChkSumOfDataPkt(ref CommBuf, HEADER_SIZE + DEV_ID_SIZE + nSize);
	
	        if( wChkSum != wReceivedChkSum ) 
		        return PKT_CHK_SUM_ERR;
	        return 0;
        }

        WORD CalcChkSumOfCmd( ref CMD_PKT pPkt )
        {
	        WORD wChkSum = 0;
            BYTE[] Buf = PktToByteArray(ref pPkt);
	        int i;
	
	        for(i=0;i<(PKT_SIZE-CHK_SUM_SIZE);i++)
		        wChkSum += Buf[i];
	        return wChkSum;
        }

        WORD CalcChkSumOfDataPkt( ref BYTE[] DataPkt, int nSize )
        {
	        int i;
	        WORD wChkSum = 0;
	
	        for(i=0;i<nSize;i++)
                wChkSum += DataPkt[i];
	        return wChkSum;
        }

        BYTE[] PktToByteArray(ref CMD_PKT package)
        {
            byte[] arr = new byte[12]; // 12 = sizeof(CMD_PKT)

            arr[0] = package.Head1;
            arr[1] = package.Head2;
            arr[2] = package.wDevId0;
            arr[3] = package.wDevId1;
            arr[4] = package.nParam0;
            arr[5] = package.nParam1;
            arr[6] = package.nParam2;
            arr[7] = package.nParam3;
            arr[8] = package.wCmd0;
            arr[9] = package.wCmd1;
            arr[10] = package.wChkSum0;
            arr[11] = package.wChkSum1;

            return arr;
        }

        CMD_PKT ByteArrayToPkt(ref BYTE[] arr)
        {
            CMD_PKT package = new CMD_PKT();

            package.Head1 = arr[0];
            package.Head2 = arr[1];
            package.wDevId0 = arr[2];
            package.wDevId1 = arr[3];
            package.nParam0 = arr[4];
            package.nParam1 = arr[5];
            package.nParam2 = arr[6];
            package.nParam3 = arr[7];
            package.wCmd0 = arr[8];
            package.wCmd1 = arr[9];
            package.wChkSum0 = arr[10];
            package.wChkSum1 = arr[11];

            return package;
        }
    }

    public struct CMD_PKT
    {
        public byte Head1;
        public byte Head2;
        public byte wDevId0;
        public byte wDevId1;
        public byte nParam0;
        public byte nParam1;
        public byte nParam2;
        public byte nParam3;
        public byte wCmd0;
        public byte wCmd1;
        public byte wChkSum0;
        public byte wChkSum1;
    }
}
