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
#include <iostream>
#include <fstream>
#include <conio.h>

#include "gt511c1.h"
#include "Console.h"

using namespace std;

CGT511C1 *fpUnit;

void LedOn()
{
	fpUnit->cmosLed(true);
}

void LedOff()
{
	fpUnit->cmosLed(false);
}

bool DeleteAll()
{
	fpUnit->deleteAll();
	if (fpUnit->gwLastAckParam == 0)
		return true;
	return false;
}

bool IsPressFinger()
{
	fpUnit->cmosLed(true);

	cout << endl;
	cout << "Put your finger in the module and Press any key to Continue" << endl;
    system("pause > nul");

	fpUnit->isPressFinger();
	if (fpUnit->gwLastAckParam == 0) {
		fpUnit->cmosLed(false);
		return true;
	}
	fpUnit->cmosLed(false);
	return false;
}

bool EnrollFingerprint(int ID)
{
	fpUnit->cmosLed(true);
	fpUnit->enroll_start(ID);

	cout << "Put your finger in the module and Press any key to Capture the fingerprint (1/3)" << endl;
    system("pause > nul");

	fpUnit->captureFinger(true);
	fpUnit->enroll_num(1);

	cout << "Put your finger in the module and Press any key to Capture the fingerprint (2/3)" << endl;
    system("pause > nul");

	fpUnit->captureFinger(true);
	fpUnit->enroll_num(2);

	cout << "Put your finger in the module and Press any key to Capture the fingerprint (3/3)" << endl;
    system("pause > nul");

	fpUnit->captureFinger(true);
	fpUnit->enroll_num(3);

	fpUnit->cmosLed(false);

	cout << "Ready." << endl;
	return true;
}

int Identify()
{
	int response;

	fpUnit->cmosLed(true);

	cout << endl;
	cout << "Put your finger in the module and Press any key to Continue" << endl;
    system("pause > nul");

	fpUnit->captureFinger(true);
	fpUnit->identify();
	if (fpUnit->gwLastAck == CMD_ACK)
		response = fpUnit->gwLastAckParam;
	else
		response = -1;
	fpUnit->cmosLed(false);

	return response;
}

int MakeTemplate()
{
	int response;

	cout << "LedOn Command" << endl;
	fpUnit->cmosLed(true);

	cout << "Put your finger in the module and Press any key to continue" << endl;
    system("pause > nul");

	fpUnit->captureFinger(true);

	if (fpUnit->makeTemplate()!=COMM_ERR) {
		if (fpUnit->gwLastAck == CMD_ACK)
			response = fpUnit->gwLastAckParam;
		else
			response = -1;
		fpUnit->cmosLed(false);

		return response;
	}

	cout << "Error Making Template" << endl;
	fpUnit->cmosLed(false);

	return COMM_ERR;
}

void GetTemplate(int Id)
{
	if (fpUnit->getTemplate(Id)==COMM_ERR) 
		cout << "Error Getting Template" << endl;
}

bool Verify(int nId)
{
	bool response = false;

	fpUnit->cmosLed(true);

	printf("Put your finger in the module and Press any key to continue\n");
    system("pause > nul");

	fpUnit->captureFinger(true);
	fpUnit->verify(nId);
	if (fpUnit->gwLastAck == CMD_ACK)
		response = true;
	fpUnit->cmosLed(false);

	return response;
}

int GetNumFingerprints()
{
	fpUnit->getEnrollCount();
	return fpUnit->gwLastAckParam;
}

int _tmain(int argc, _TCHAR* argv[]) {
	char result;
	int iTmp;
	
	system("cls");
	cout << "Fingerprint Module GT511C1" << endl << endl;
	cout << "Introduce the Number of COMM port and press enter:";
	cin >> iTmp;

	fpUnit = new CGT511C1(iTmp,9600);
	if (fpUnit->status == false) {
		cout << "Counldn't open the Fingerprint Module" << endl;
		delete(fpUnit);
		exit(1);
	}
	result = 0;
	do {
		cout << "******* Fingerprint Module GT511C1 ********" << endl << endl;
		cout << "1) Get num fingerprints " << endl;
		cout << "2) Led On" << endl;
		cout << "3) Led Off" << endl;
		cout << "4) Enroll Fingerprint" << endl;
		cout << "5) Identify User" << endl;
		cout << "6) Verify User" << endl;
		cout << "7) IsPressFinger" << endl;
		cout << "8) Delete All Fingerprints" << endl;
		cout << "9) exit" << endl;

		result = getche();

		switch(result) {
			case '1':
				cout << endl << endl << GetNumFingerprints() << " fingerprints in module" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '2':
				LedOn();
				cout << endl << endl << "Led Module On" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '3':
				LedOff();
				cout << endl << endl << "Led Module Off" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '4':
				iTmp = GetNumFingerprints();
				EnrollFingerprint(iTmp);
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '5':
				iTmp = Identify();
				cout << endl << endl << "User " << iTmp << " identified" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '6':
				cout << endl << endl << "Introduce the User Id to Verify and press enter to continue:";
				cin >> iTmp;
				if (Verify(iTmp)) {
					cout << "User correctly verified" << endl;
					cout << "Press any key to continue";
					system("pause > nul");
					system("cls");
					break;
				}
				cout << "User rejected" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
			case '7':
				cout << endl;
				if(IsPressFinger())
					cout << "Finger present" << endl;
				else
					cout << "Finger not found" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '8':
				cout << endl << endl << "You are about to delete fingerprints in the module are you sure? (y/n):";
				result = getche();
				cout << endl;
				if (result == 'y' || result == 'Y') {
					DeleteAll();
					cout << "Fingerprints deleted" << endl;
					cout << "Press any key to continue";
					system("pause > nul");
					system("cls");
					break;
				}
				cout << "Operation aborted" << endl;
				cout << "Press any key to continue";
				system("pause > nul");
				system("cls");
				break;
			case '9':
				exit(0);
				break;
			default:
				system("cls");
				break;
		} 
	}while(true);
	delete(fpUnit);
	return 0;
}

//MakeTemplate(); 
//GetTemplate(1);  
//fpUnit.getImage();
//fpUnit.getRawImage();
//GetImage();
//Compare();