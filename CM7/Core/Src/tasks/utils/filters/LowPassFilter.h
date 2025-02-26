/*
 * LowPassFilter.h
 *
 *  Created on: Feb 24, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_LOWPASSFILTER_H_
#define SRC_TASKS_UTILS_LOWPASSFILTER_H_

#include <array>
#include <inttypes.h>

#include "FreeRTOS.h"
#include "task.h"

#include "DigitalFilter.h"

#define LOWPASSFILTER_SIZE 5

class LowPassFilter : public DigitalFilter
{
	public:
		LowPassFilter(float tau) : DigitalFilter(LOW_PASS)
		{
			mTau = tau;
			buffer.fill(0);
			mIndex = 0;
		}

	    void update() override
	    {

	    	uint32_t dt = xTaskGetTickCount() - mPreviousTick;
	    	mPreviousTick = xTaskGetTickCount();
	        // Update circular buffer
	        buffer[mIndex] = mInput;
	        mIndex = (mIndex + 1) % buffer.size();

	        // Compute simple moving average (SMA)
	        float sum = 0;
	        for (float val : buffer)
	            sum += val;

	        float sma = sum / buffer.size();

	        // Compute adaptive smoothing factor
	        float alpha = dt / (mTau + dt);

	        // Apply exponential smoothing on SMA
	        int32_t res = alpha * sma + (1 - alpha) * mOutput;

			if (res != mOutput)
			{
				mNewOutput = true;
				mOutput = res;
			}
	    }


	private:
		float mTau;        // Time constant
		bool mInitialized; // To handle first update
		uint32_t mPreviousTick;
		uint8_t mIndex;
		std::array<float, LOWPASSFILTER_SIZE> buffer;
};

#endif /* SRC_TASKS_UTILS_LOWPASSFILTER_H_ */
