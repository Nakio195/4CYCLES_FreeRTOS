/*
 * Switch.h
 *
 *  Created on: 13 avr. 2022
 *      Author: TooT
 */

#ifndef DRIVERS_SWITCH_H_
#define DRIVERS_SWITCH_H_

#include <stdint.h>

class Switch
{
	public:

		enum States
		{
			RELEASED,
			PRESSED,
			REPEAT,
			IDLE
		};

	public:
		Switch();

		void update(bool newState);
		States read();

		bool isPressed()
		{
			return read() == PRESSED;
		}

		bool isReleased()
		{
			return read() == RELEASED;
		}

		bool isReapeat()
		{
			return read() == REPEAT;
		}

		//Configuration
		void setRepeat(uint16_t delay);
		void setDebounce(uint16_t debounce);
		void setRepeatTime(uint16_t time);

		void tick(uint16_t dt);


	private :
		bool mCurrentState;
		uint8_t mPreviousState;

		States mNewState;

		uint16_t mDebounce;
		uint16_t mDebounceTime;
		uint16_t mDebounceTimer;

		bool mRepeat;
		uint16_t mRepeatDelay;
		uint16_t mRepeatTime;
		uint16_t mRepeatTimer;
};

#endif /* DRIVERS_SWITCH_H_ */
