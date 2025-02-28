/*
 * ThresholdFilter.h
 *
 *  Created on: Feb 27, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_FILTERS_THRESHOLDFILTER_H_
#define SRC_TASKS_UTILS_FILTERS_THRESHOLDFILTER_H_

#include <vector>
#include <tuple>

#include "DigitalFilter.h"

class ThresholdFilter: public DigitalFilter
{
	public:
		ThresholdFilter() : DigitalFilter(THRESHOLD)
		{
			mThresholds.clear();
		}

		void addThreshold(int low, int high, int output)
		{
			mThresholds.push_back(std::make_tuple(low, high, output));
		}

		void update() override
		{
			for (const auto& threshold : mThresholds)
			{
				int low, high, output;

				std::tie(low, high, output) = threshold;

				if (mInput >= low && mInput <= high)
				{
					mOutput = output;
					mNewOutput = true;
					return;
				}
			}
		}

	protected:
		std::vector<std::tuple<int, int, int>> mThresholds;
};


#endif /* SRC_TASKS_UTILS_FILTERS_THRESHOLDFILTER_H_ */
