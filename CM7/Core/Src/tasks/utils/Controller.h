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

#define ACTION_POOL_SIZE 500

class Action
{
	public:
		enum Type{Invalid, Throttle, Brake, Steering, Motor, Lights, Horn, Controller, Engage};
		enum Gear{Slow, Middle, Fast, Reverse, Forward};
		enum Signals{Left, Right, BrakeSignal, NighLight, Hazard};
		enum ControllerEvent{Absent, Connected};

	public:
		Action(Type type = Invalid)
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

		Action::Gear getReverse()
		{
			return Action::Gear(mValues[0]);
		}

		bool getLightState()
		{
			return mValues[1];
		}

		void reset(Type type)
		{
			mType = type;
			mTimestamp = xTaskGetTickCount();
			mValues.clear();
		}

	private:
		Type mType;
		uint32_t mTimestamp;
		std::vector<uint32_t> mValues;

};

class ActionPacketPoolHandler
{
	private:
		Action pool[ACTION_POOL_SIZE];
		bool used[ACTION_POOL_SIZE];        // Indique si le slot est utilisé
		SemaphoreHandle_t mutex;

		uint16_t minPoolSizeEver;
		uint16_t currentPoolUse;

	public:
		ActionPacketPoolHandler()
		{
			mutex = xSemaphoreCreateMutex();
			for (int i = 0; i < ACTION_POOL_SIZE; i++)
				used[i] = false;
			minPoolSizeEver = ACTION_POOL_SIZE; // Initialisation à la taille maximale
			currentPoolUse = 0;
		}

		Action* allocate(Action::Type type)
		{
			Action* pkt = nullptr;
			if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
			{
				for (int i = 0; i < ACTION_POOL_SIZE; i++)
				{
					if (!used[i])
					{
						used[i] = true;
						pkt = &pool[i];
						break;
					}
				}
				xSemaphoreGive(mutex);
			}
			if(pkt != nullptr)
				pkt->reset(type);

			assert(pkt != nullptr); // Ensure that the request was allocated successfully
			currentPoolUse++;
			if (ACTION_POOL_SIZE - currentPoolUse < minPoolSizeEver)
				minPoolSizeEver = ACTION_POOL_SIZE - currentPoolUse;
			return pkt; // nullptr si pool plein
		}

		void free(Action* pkt)
		{
			if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
				int index = pkt - pool; // calcul index
				if (index >= 0 && index < ACTION_POOL_SIZE) {
					used[index] = false;
				}
				xSemaphoreGive(mutex);
				currentPoolUse--;
			}
		}
};


extern ActionPacketPoolHandler ActionPacketPool;


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
			Action* action = ActionPacketPool.allocate(type);
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
