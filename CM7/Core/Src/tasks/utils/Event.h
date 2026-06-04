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

		struct PeripheralEvent
		{
			uint8_t type;
			uint8_t id;
		};

	public:
		enum EventType : uint8_t {
			PeripheralDiscover,
			PeripheralReady,
			PeripheralEnabled,
			PeripheralDisabled,
			PeripheralMissing,
			PeripheralLost,
			PeripheralRecovered
		};

		EventType type;

		union
		{
			PeripheralEvent peripheral;
		};
};

#endif /* SRC_TASKS_UTILS_EVENT_H_ */
