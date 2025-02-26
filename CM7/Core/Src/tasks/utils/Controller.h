/*
 * Controller.h
 *
 *  Created on: Feb 21, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_CONTROLLER_H_
#define SRC_TASKS_UTILS_CONTROLLER_H_

#include <vector>
#include "FreeRTOS.h"
#include "queue.h"


class Action
{
	public:
		enum Type{Throttle, Brake, Steering, Gear, Lights, Horn};
		enum Gear{Slow, Middle, Fast};
		enum Lights{Off, Left, Right, Star};

	public:
		Action(Type type)
		{
			mType = type;
			mTimestamp = xTaskGetTickCount();
		}

		Type inline type()
		{
			return mType;
		}

		void inline push(uint32_t value)
		{
			mValues.push_back(value);
		}

		uint8_t inline getThrottleValue()
		{
			return mValues[0];
		}

		uint8_t inline getBrakeValue()
		{
			return mValues[0];
		}

	private:
		Type mType;
		uint32_t mTimestamp;
		std::vector<uint32_t> mValues;

};

class Controller
{
	public:
		Controller()
		{
			mQueue = xQueueCreate(30, sizeof(Action));
		}

		void setThrottleCommand(uint8_t value)
		{
			if (mThrottle != value)
			{
				mThrottle = value;
				mThrottleChanged = true;
				pushAction(Action::Throttle, value);
			}

			else
				mThrottleChanged = false;
		}

		void setBrakeCommand(uint8_t value)
		{
			if (mBrake != value)
			{
				mBrake = value;
				mBrakeChanged = true;
				pushAction(Action::Brake, value);
			}

			else
				mBrakeChanged = false;
		}

		void inline pushAction(Action::Type type, uint32_t value)
		{
			Action* action = new Action(type);
			action->push(value);
			xQueueSend(mQueue, &action, 0);
		}

		void inline pushAction(Action *action)
		{
			xQueueSend(mQueue, &action, 0);
		}

		QueueHandle_t inline getQueue()
		{
			return mQueue;
		}

	protected:
		uint8_t mThrottle;
		bool mThrottleChanged;

		uint8_t mBrake;
		bool mBrakeChanged;

		int8_t mSteering;
		bool mSteeringChanged;


		QueueHandle_t mQueue;

};

#endif /* SRC_TASKS_UTILS_CONTROLLER_H_ */
