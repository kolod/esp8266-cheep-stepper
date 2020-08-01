// Copyright (C) 2020  Oleksandr Kolodkin <alexandr.kolodkin@gmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include <CheapStepper.h>

// pin #          ULN2003 pin
// 14      >--<   IN1
// 12      >--<   IN2
// 13      >--<   IN3
// 15      >--<   IN4

int seq = 0;

void setup() {

	// let's just set up a serial connection and test print to the console
	Serial.begin(9600);
	Serial.println("Ready to start moving!");

	// Init stepper
	Stepper.init(14, 12, 13, 15, 10, 4096);
}

void loop() {
	if (Stepper.isReady()) switch(seq) {
		case 0:
			Serial.println("Move CW 90");
			Stepper.moveDegreesCW(90);
			seq = 1;
			break;
		
		case 1:
			Serial.println("Move CCW 90");
			Stepper.moveDegreesCCW(90);
			seq = 2;
			break;
		
		case 2:
			Serial.println("Move CW 360");
			Stepper.moveDegreesCW(360);
			seq = 3;
	}
}