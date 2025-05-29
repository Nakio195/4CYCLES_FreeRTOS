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
		enum Type{Throttle, Brake, Steering, Gear, Lights, Horn, Controller};
		enum Gear{Slow, Middle, Fast};
		enum Signals{Left, Right, BrakeSignal, NighLight, Hazard};
		enum ControllerEvent{Absent, Connected};

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

		int32_t inline getSteeringValue()
		{
			return mValues[0];
		}

		Action::Signals getLightsType()
		{
			return Action::Signals(mValues[0]);
		}

		bool getLightState()
		{
			return mValues[1];
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
			mQueue = xQueueCreate(100, sizeof(Action*));
		}

		void setThrottleCommand(uint8_t value)
		{
			pushAction(Action::Throttle, value);
		}

		void setBrakeCommand(uint8_t value)
		{
			pushAction(Action::Brake, value);
		}

		void setSteeringCommand(int32_t value)
		{
			pushAction(Action::Steering, value);
		}

		void setLightsCommand(Action::Signals light, uint8_t value)
		{
			Action* action = new Action(Action::Lights);
			action->push(uint32_t(light));
			action->push(value);
			pushAction(action);
		}


		void setControllerEvent(uint8_t event)
		{
			pushAction(Action::Controller, uint32_t(event));
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


		QueueHandle_t mQueue;

};

#endif /* SRC_TASKS_UTILS_CONTROLLER_H_ */
