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


#include "Arduino.h"
#include "CheapStepper.h"

extern "C" void CheapStepperTimerISR() {
	Stepper.step();
}
 
CheapStepper::CheapStepper (){
	mIsStopped = true;
}

void CheapStepper::step()
{
	const uint8_t pattern[4] = {
		0b10000011, 
		0b00111000,
		0b00001110,
		0b11100000
	};

	// write pattern to pins
	for (int i = 0; i < 4; i++)  digitalWrite(mPins[i], pattern[i] & mStepMask);

	// calculate next pattern
	if (!mIsStopped) {
		if (mSetpoint > mPosition) {
			mPosition++;
			if (mPosition >= mLimitCW) {
				mIsStopped = true;
				mLastMask = mStepMask;
				mStepMask = 0;
			} else {
				mStepMask <<= 1;
				if (mStepMask == 0) mStepMask = 0b00000001;
			}
		} else if (mSetpoint < mPosition) {
			mPosition--;
			if (mPosition >= mLimitCCW) {
				mIsStopped = true;
				mLastMask = mStepMask;
				mStepMask = 0;
			} else {
				mStepMask >>= 1;
				if (mStepMask == 0) mStepMask = 0b10000000;
			}
		}

		if (mSetpoint == mPosition) {
			mIsStopped = true;
			mLastMask = mStepMask;
			mStepMask = 0;
		}
	}
}

CheapStepper Stepper;
