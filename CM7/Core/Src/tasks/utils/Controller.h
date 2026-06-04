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
#include "Action.h"


class Controller
{
	public:
		Controller()
		{
			mQueue = xQueueCreate(100, sizeof(Action*));
			mActive = false;
		}

		void activate()
		{
			mActive = true;
		}

		void deactivate()
		{
			mActive = false;
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

		void setMotorEngage(uint8_t value)
		{
			pushAction(Action::Engage, value);
		}


		void setLightsCommand(Action::Signals light, uint8_t value)
		{
			Action* action = ActionPacketPool.allocate(Action::Lights);
			action->push(uint32_t(light));
			action->push(uint32_t(value));
			pushAction(action);
		}

		void setReverseCommand(Action::Gear gear)
		{
			Action *action = ActionPacketPool.allocate(Action::Type::Motor);
			action->push(uint32_t(gear));
			pushAction(action);
		}


		void setControllerEvent(uint8_t event)
		{
			pushAction(Action::Controller, uint32_t(event));
		}

		void inline pushAction(Action::Type type, uint32_t value)
		{
			if(!mActive)
				return;

			Action* action = ActionPacketPool.allocate(type);
			action->push(value);
			xQueueSend(mQueue, &action, 0);
		}

		void inline pushAction(Action *action)
		{
			if(!mActive)
			{
				ActionPacketPool.free(action);
				return;
			}

			xQueueSend(mQueue, &action, 0);
		}

		QueueHandle_t inline getControllerQueue()
		{
			return mQueue;
		}


	protected:
		QueueHandle_t mQueue;
		bool mActive; 	// Indicate that it should generate Actions

};

#endif /* SRC_TASKS_UTILS_CONTROLLER_H_ */
