/*
 * BidirectionnalQueue.h
 *
 *  Created on: Mar 3, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_BIDIRECTIONNALQUEUE_H_
#define SRC_TASKS_UTILS_BIDIRECTIONNALQUEUE_H_

#include "FreeRTOS.h"
#include "queue.h"

#include <inttypes.h>
#include <string>

#define SHARED_RECEIVE_QUEUE_ADDRESS uint32_t(0x30020000)
#define SHARED_SEND_QUEUE_ADDRESS uint32_t(SHARED_RECEIVE_QUEUE_ADDRESS + 0xFF)

template <typename T>
class BidirectionnalQueue
{
	public:
		BidirectionnalQueue(size_t size, std::string name = "", bool owner = true)
	    {
	        if(!owner)
	        {
	            // Use the shared handles if provided
	        	//Swappoing Receive and send queues so that the M4 can send data to the M7
	            mQueueSend = (QueueHandle_t)(SHARED_RECEIVE_QUEUE_ADDRESS);
	            mQueueReceive = (QueueHandle_t)(SHARED_SEND_QUEUE_ADDRESS);
	        }

	        else
	        {
	            // Create the queues
	            mQueueSend = xQueueCreate(size, sizeof(T));
	            mQueueReceive = xQueueCreate(size, sizeof(T));

	            if (!name.empty())
	            {
	                vQueueAddToRegistry(mQueueSend, std::string(name + " M7 Send").c_str());
	                vQueueAddToRegistry(mQueueReceive, std::string(name + " M7 Receive").c_str());
	            }
	        }
	    }

		bool send(T data, TickType_t timeout = portMAX_DELAY)
		{
			return xQueueSend(mQueueSend, &data, timeout) == pdTRUE;
		}

		T receive(TickType_t timeout = portMAX_DELAY)
		{
			T data;
			xQueueReceive(mQueueReceive, &data, timeout);
			return data;
		}

		uint8_t available()
		{
			return uxQueueMessagesWaiting(mQueueReceive) > 0;
		}

		~BidirectionnalQueue()
		{
			vQueueDelete(mQueueSend);
			vQueueDelete(mQueueReceive);
		}

	private:
		QueueHandle_t mQueueSend;
		QueueHandle_t mQueueReceive;
};

#endif /* SRC_TASKS_UTILS_BIDIRECTIONNALQUEUE_H_ */
