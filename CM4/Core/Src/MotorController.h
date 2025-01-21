/*
 * MotorController.h
 *
 *  Created on: Jan 16, 2025
 *      Author: To
 */

#ifndef MOTORCONTROLLER_H_
#define MOTORCONTROLLER_H_


#include <vector>
#include "utils/Phaserunner.hpp"

class MotorController
{
	public:
		MotorController();
		virtual ~MotorController();

		static void tick();
		void process();
		void toggleRed();

		Phaserunner *Motor;
};

#endif /* MOTORCONTROLLER_H_ */
