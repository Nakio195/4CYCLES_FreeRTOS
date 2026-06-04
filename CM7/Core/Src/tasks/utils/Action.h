/*
 * Action.h
 *
 *  Created on: May 28, 2026
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_ACTION_H_
#define SRC_TASKS_UTILS_ACTION_H_


#define ACTION_POOL_SIZE 50

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

		//Generic
		Type inline type()
		{
			return mType;
		}

		void inline push(uint32_t value)
		{
			//TODO Check array size before
			mValues.push_back(value);
		}

		void reset(Type type)
		{
			mType = type;
			mTimestamp = xTaskGetTickCount();
			mValues.clear();
		}

		// Controller Oriented
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
			if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
			{
				int index = pkt - pool; // calcul index
				if (index >= 0 && index < ACTION_POOL_SIZE)
				{
					used[index] = false;
				}
				xSemaphoreGive(mutex);
				currentPoolUse--;
			}
		}
};


extern ActionPacketPoolHandler ActionPacketPool;




#endif /* SRC_TASKS_UTILS_ACTION_H_ */
