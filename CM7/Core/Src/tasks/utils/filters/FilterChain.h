/*
 * FilterChain.h
 *
 *  Created on: Feb 26, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_FILTERCHAIN_H_
#define SRC_TASKS_UTILS_FILTERCHAIN_H_

#include <inttypes.h>
#include "DigitalFilter.h"
#include <vector>


class FilterChain
{
	public:
		FilterChain();

		void addFilter(DigitalFilter* filter);
		void update();
		void setInput(int32_t value);
		int32_t getOutput();

		bool hasChanged()
		{
			return mFilters.back()->hasChanged();
		}

	private:
		std::vector<DigitalFilter*> mFilters;
};

#endif /* SRC_TASKS_UTILS_FILTERCHAIN_H_ */
