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

#include <SoftwareSerial.h>
#include "gt511c1.h"

SoftwareSerial vport(2, 3); // RX, TX
CGT511C1 fpUnit(&vport, 9600);

void LedOn()
{
  fpUnit.cmosLed(true);
}

void LedOff()
{
  fpUnit.cmosLed(false);
}

void setup() {     
  LedOn();  
}

void loop() {
}
