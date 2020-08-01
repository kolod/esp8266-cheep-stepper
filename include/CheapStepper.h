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

#ifndef CHEAPSTEPPER_H
#define CHEAPSTEPPER_H

#include "Arduino.h"
#include "hw_timer.h"

extern "C" void CheapStepperTimerISR();

class CheapStepper
{

public: 
	CheapStepper();

	void init(const int pin1, const int pin2, const int pin3, const int pin4, int spr, int rpm) {
		mPins[0] = pin1;
		mPins[1] = pin2;
		mPins[2] = pin3;
		mPins[3] = pin4;
		mSpr = spr;
		mRpm = rpm;

		mIsStopped = 0;
		mStepMask = 0;
		mLastMask = 1;
		mLimitCW = INT32_MAX;
		mLimitCCW = INT32_MIN;

		if (mRpm < 6 ) mRpm = 6;  // if rpm <6 will overheat
		if (mRpm > 23) mRpm = 23; // highest speed
	
		initTimer();
	};

	// Move spesified steps clockwise
	void moveCW(int32_t value) {
		auto setpoint = mPosition + value;
		if (setpoint > mLimitCW) setpoint = mLimitCW;
		if (mPosition < setpoint) {
			noInterrupts();
			mIsStopped = false;
			mSetpoint = setpoint;
			mStepMask = mLastMask;
			interrupts();
		}
	}

	// Move spesified steps counterclockwise
	void moveCCW(int32_t value) {
		auto setpoint = mPosition - value;
		if (setpoint < mLimitCCW) setpoint = mLimitCCW;
		if (mPosition > setpoint) {
			noInterrupts();
			mIsStopped = false;
			mSetpoint = setpoint;
			mStepMask = mLastMask;
			interrupts();
		}
	}

	// Move in spesified degree
	void moveDegreesCW(int32_t value)  {moveCW(degreeasToSteps(value));}
	void moveDegreesCCW(int32_t value) {moveCCW(degreeasToSteps(value));}

	// Move to spesified position clockwise
	void moveTo(int32_t value) {
		if (mIsStopped && (value > mLimitCCW) && (value < mLimitCW)) {
			noInterrupts();
			mIsStopped = false;
			mSetpoint = value;
			mStepMask = mLastMask;
			interrupts();
		}
	}

	void moveToDegree(int32_t value) {
		moveTo(degreeasToSteps(value));
	}

	void off() {
		mIsStopped = true;
		mSetpoint = mPosition;
	}

	void stepCW()  {moveCW(1);}    // move 1 step clockwise
	void stepCCW() {moveCCW(1);}   // move 1 step counter-clockwise

	int position()  const {return mPosition;}  // returns current position in steps
	int setpoint()  const {return mSetpoint;}
	int rpm()       const {return mRpm;}
	int spr()       const {return mSpr;}

	// returns steps left in current move 
	int stepsLeft() const {
		if (mSetpoint > mPosition) {
			return mSetpoint - mPosition;
		} else {
			return mPosition - mSetpoint;
		}
	} 

	// returns true if moving done
	bool isReady() const {return mIsStopped;}

	// set virtual limit switches
	void setLimits(uint32_t cw, uint32_t ccw) {
		if (mIsStopped && (cw > ccw)) {
			mLimitCW  = cw;
			mLimitCCW = ccw;
		}
	}

	void setLimitsDegree(uint32_t cw, uint32_t ccw) {
		setLimits(degreeasToSteps(cw), degreeasToSteps(ccw));
	}

	// reset current position to zero
	void resetPosition() {
		if (mIsStopped) mSetpoint = mPosition = 0;
	}
 
	// send step sequence to driver
	void step();

private:
	int mPins[4];             // defaults to pins {8,9,10,11} (in1,in2,in3,in4 on the driver board)
	int32_t mPosition;        // current position in steps
	int32_t mSetpoint;        // requred position in steps
	uint16_t mSpr;            // steps per revolution
	uint16_t mRpm;            // revolutions per minute
	int32_t mLimitCW;         // virtual limit switch for CW
	int32_t mLimitCCW;        // virtual limit switch for CCW
	uint8_t mStepMask;        // microstep mask
	uint8_t mLastMask;        // saved mask
	bool mIsStopped;          // is motor stopped

	void initTimer() {
		hw_timer_init(NMI_SOURCE, 1);
		hw_timer_set_func(CheapStepperTimerISR);
		hw_timer_arm(static_cast<uint64_t>(60000000) / (static_cast<uint64_t>(mRpm) * static_cast<uint64_t>(mSpr)));
	}

	int32_t degreeasToSteps(const int32_t value) const {
		return static_cast<int64_t>(value) * mSpr / 360;
	}

	int32_t StepsToDegreeas(const int32_t value) const {
		return static_cast<int64_t>(value) * 360 / mSpr;
	}
};

extern CheapStepper Stepper;

#endif
