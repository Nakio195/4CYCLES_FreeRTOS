/*
 * FilterChain.cpp
 *
 *  Created on: Feb 26, 2025
 *      Author: To
 */

#include "FilterChain.h"

FilterChain::FilterChain()
{
	// TODO Auto-generated constructor stub

}

void FilterChain::addFilter(DigitalFilter *filter)
{
	mFilters.push_back(filter);
}

void FilterChain::update()
{
	for (uint8_t i = 0; i < mFilters.size(); i++)
	{
		if(i == 0)
			mFilters[i]->update();
		else
		{
			mFilters[i]->setInput(mFilters[i - 1]->getOutput());
			mFilters[i]->update();
		}
	}
}

void FilterChain::setInput(int32_t value)
{
	mFilters.front()->setInput(value);
}

int32_t FilterChain::getOutput()
{
	return mFilters.back()->getOutput();
}



