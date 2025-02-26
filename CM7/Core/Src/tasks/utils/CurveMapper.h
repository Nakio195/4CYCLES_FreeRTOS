 /*
 * CurveMapper.h
 *
 *  Created on: Feb 21, 2025
 *      Author: To
 */

#ifndef SRC_TASKS_UTILS_CURVEMAPPER_H_
#define SRC_TASKS_UTILS_CURVEMAPPER_H_


#include <vector>
#include <algorithm>
#include "ArduinoJson-v7.3.0.h"

class CurveMapper
{
	public:
		CurveMapper();

		void inline addPoint(float input, float output)
		{
			points.emplace_back(input, output);
			std::sort(points.begin(), points.end());
		}

		void inline clear()
		{
			points.clear();
		}

		std::vector<std::pair<float, float>> inline getPoints() const
		{
			return points;
		}

		void inline setPoints(JsonArray data)
		{
			points.clear();
			for (JsonVariant point : data)
			{
				addPoint(point[0], point[1]);
			}
		}

		float inline map(float input) const
		{
			if (points.empty())
			{
				return 0.0f; // or some default value
			}

			if (input <= points.front().first)
			{
				return points.front().second;
			}

			if (input >= points.back().first)
			{
				return points.back().second;
			}

			for (size_t i = 1; i < points.size(); ++i)
			{
				if (input < points[i].first)
				{
					float t = (input - points[i - 1].first) / (points[i].first - points[i - 1].first);
					return points[i - 1].second + t * (points[i].second - points[i - 1].second);
				}
			}

			return 0.0f; // should never reach here
		}

	private:
		std::vector<std::pair<float, float>> points;
};


#endif /* SRC_TASKS_UTILS_CURVEMAPPER_H_ */
