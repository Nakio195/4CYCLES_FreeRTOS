/*
 * DigitalFilter.h
 *
 *  Created on: Feb 26, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_DIGITALFILTER_H_
#define SRC_TASKS_UTILS_DIGITALFILTER_H_

#include <inttypes.h>

class DigitalFilter
{

	public:
		enum FilterType
		{
			LOW_PASS, SCURVE, THRESHOLD
		};

	public:
		DigitalFilter(FilterType type) : mType(type)
		{
			mInput = 0;
			mOutput = 0;
			mNewInput = false;
			mNewOutput = false;
		}

		virtual void update() = 0;

		virtual void setInput(float value)
		{
			if (value != mInput)
			{
				mInput = value;
				mNewInput = true;
			}

			else
			{
				mNewInput = false;
			}
		}

		float getOutput()
		{
			mNewOutput = false;
			return mOutput;
		}

		bool hasChanged()
		{
			return mNewOutput;
		}


		protected:
			float mInput;
			float mOutput;

			bool mNewInput;
			bool mNewOutput;
        	FilterType mType;

};

#endif /* SRC_TASKS_UTILS_DIGITALFILTER_H_ */
