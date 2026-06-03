/*
 * Event.h
 *
 *  Created on: Jun 3, 2026
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_EVENT_H_
#define SRC_TASKS_UTILS_EVENT_H_

class Event
{
	public:
//		Event() ;

		struct PeripheralDiscoveredEvent
		{
			uint8_t type;
			uint8_t id;
		};

		struct PeripheralConnectedEvent
		{
			uint8_t type;
			uint8_t id;
		};

		struct PeripheralDisconnectedEvent
		{
			uint8_t type;
			uint8_t id;
		};


	public:
		enum EventType : uint8_t {
			PeripheralDiscover,
			PeripheralConnect,
			PeripheralDisconnect
		};

		EventType type;

		union
		{
			PeripheralDiscoveredEvent PeripheralDiscovered;
			PeripheralConnectedEvent PeripheralConnected;
			PeripheralDisconnectedEvent PeripheralDisconnected;
		};
};

#endif /* SRC_TASKS_UTILS_EVENT_H_ */
