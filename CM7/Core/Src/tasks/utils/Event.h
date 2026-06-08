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

		struct Peripheral
		{
			enum PeripheralEventType : uint8_t {
				Discover,
				Ready,
				Enabled,
				Disabled,
				Missing,
				Lost,
				Recovered
			};

			uint8_t type; //Type de périphérique
			PeripheralEventType event; // Type d'évenement
			uint8_t id; // Id du périphérique
		};

		struct Controller
		{
			enum ControllerEventType : uint8_t {
				Change
			};

			ControllerEventType request;
		};

	public:
		enum EventType : uint8_t {
			PeripheralEvent,
			ControllerEvent
		};

		EventType type;

		union
		{
			Peripheral peripheral;
			Controller controller;
		};
};

#endif /* SRC_TASKS_UTILS_EVENT_H_ */
