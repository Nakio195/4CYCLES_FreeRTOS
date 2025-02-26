/*
 * SCurveFilter.h
 *
 *  Created on: Feb 24, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_SCURVEFILTER_H_
#define SRC_TASKS_UTILS_SCURVEFILTER_H_

#include <math.h>
#include <inttypes.h>

#include "FreeRTOS.h"
#include "task.h"

#include "DigitalFilter.h"

class SCurveFilter : public DigitalFilter
{
	public:
		SCurveFilter(float start = 0, float target = 0, float steepness = 10.0f) : DigitalFilter(SCURVE)
		{
			startValue = start;
			targetValue = target;
			k = steepness;
			updateDuration(); // Initialize totalDuration
			currentTime = 0.0f;
		}

		// Update the progress and adapt to the target value
		void update() override
		{

			currentTime += xTaskGetTickCount() - previousTick;
			previousTick = xTaskGetTickCount();
			// Dynamically adjust duration based on the distance to the target
			if (targetValue == startValue)
				return;

			updateDuration();

			//Clamping computation for near rails values
			if(targetValue == 0 && mOutput < 3)
				startValue = 0;
			if (targetValue == 255 && mOutput > 252)
				startValue = 255;

			// Limit currentTime to be within the duration
			if (currentTime > totalDuration)
				currentTime = totalDuration;
			// Calculate the normalized time progress (t between 0 and 1)
			float t = currentTime / totalDuration;

			// Apply the sigmoid-based S-curve formula for smooth acceleration/deceleration
			float progress = 1.0f / (1.0f + expf(float(-k * (t - 0.5f)))); // Sigmoid function centered at 0.5

			// Interpolate between start and target values based on the S-curve progress
			float res = startValue + (targetValue - startValue) * progress;

			if (res != mOutput)
			{
				mNewOutput = true;
				mOutput = res;
			}
		}

		// Update the target dynamically
		void setInput(float newTarget) override
		{
			DigitalFilter::setInput(newTarget);

			if(mNewInput)
			{
				// Update start value to the current value (before transitioning)
				startValue = mOutput;  // Fetch the current value (use update with zero deltaTime)
				targetValue = newTarget;
				currentTime = 0.0f; // Reset to start the transition again
			}
		}

	private:
		float startValue;        // initial SCurve Value
		float targetValue;       // Target value (e.g., desired motor speed)
		float totalDuration;     // Total duration for the transition
		float k;                 // Steepness of the S-curve
		float currentTime;       // Current time/progress

		uint32_t previousTick;

		// Dynamically update the duration based on the maximum acceleration and the distance to the target
		void updateDuration()
		{

			float distance = fabsf(targetValue - startValue);

			// Calculate the required duration based on the maximum acceleration:
			// Using the formula for acceleration time: t = (v - u) / a, where acceleration is capped
			if (distance != 0) {
				totalDuration = 0.0642*distance*distance;  // Using the kinematic equation with max acceleration
			} else {
				totalDuration = 0.0f;  // If there's no distance, no transition is needed
			}
		}
};

#endif /* SRC_TASKS_UTILS_SCURVEFILTER_H_ */
