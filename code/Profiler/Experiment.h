/*
 * This file is part of Micro Bench
 * <https://github.com/duckmaestro/Micro-Bench>.
 * 
 * Micro Bench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Micro Bench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include "StopwatchStack.h"
#include "Mathx.h"

class Experiment
{
public:
	Experiment(std::wstring name, StopwatchStack& stopwatch);
	virtual ~Experiment(void) { }

	virtual void Run(void) { };
	virtual std::wstring ToString(void);
	const std::wstring& GetName(void) const { return _name; };

protected:
	void PushResult(std::wstring resultName, double value);
	template<typename T> 
	void PushResult(std::wstring resultName, T value)
	{
		PushResult(resultName, (double)value);
	}

	/// <summary>
	/// Saves the mean (in μs) of a data set (in ticks) to the named result set.
	/// </summary>
	template<typename TCollection> 
	void PushResultMeanAsMicroseconds(std::wstring resultName, TCollection dataset, TickValue adjustment = 0)
	{
		StopwatchStack& ss = _stopwatch;
		double mean = Mathx::Mean<double, TickValue>(
			dataset,
			[&ss, &adjustment](TickValue t)
			{
				return ss.TicksToMicroseconds(t + adjustment);
			}
		);
		PushResult(resultName, mean);
	}

	/// <summary>
	/// Saves the median (in μs) of a data set (in ticks) to the named result set.
	/// </summary>
	template<typename TCollection>
	void PushResultMedianAsMicroseconds(std::wstring resultName, TCollection dataset, TickValue adjustment = 0)
	{
		StopwatchStack& ss = _stopwatch;
		double mean = Mathx::Median<double, TickValue>(
			dataset,
			[&ss, &adjustment](TickValue t)
			{
				return ss.TicksToMicroseconds(t + adjustment);
			}
		);
		PushResult(resultName, mean);
	}
	
	/// <summary>
	/// Saves the minimum (in μs) of a data set (in ticks) to the named result set.
	/// </summary>
	template<typename TCollection>
	void PushResultMinimumAsMicroseconds(std::wstring resultName, TCollection dataset, TickValue adjustment = 0)
	{
		StopwatchStack& ss = _stopwatch;
		double mean = Mathx::Minimum<double, TickValue>(
			dataset,
			[&ss, &adjustment](TickValue t)
			{
				return ss.TicksToMicroseconds(t + adjustment);
			}
		);
		PushResult(resultName, mean);
	}

	/// <summary>
	/// Saves the maximum (in μs) of a data set (in ticks) to the named result set.
	/// </summary>
	template<typename TCollection>
	void PushResultMaximumAsMicroseconds(std::wstring resultName, TCollection dataset, TickValue adjustment = 0)
	{
		StopwatchStack& ss = _stopwatch;
		double mean = Mathx::Maximum<double, TickValue>(
			dataset,
			[&ss, &adjustment](TickValue t)
			{
				return ss.TicksToMicroseconds(t + adjustment);
			}
		);
		PushResult(resultName, mean);
	}

	/// <summary>
	/// Saves the maximum (in μs) of a data set (in ticks) to the named result set.
	/// </summary>
	template<typename TCollection>
	void PushResultPercentileAsMicroseconds(std::wstring resultName, TCollection dataset, int percentile, TickValue adjustment = 0)
	{
		StopwatchStack& ss = _stopwatch;
		double mean = Mathx::Percentile<double, TickValue>(
			dataset,
			[&ss, &adjustment](TickValue t)
			{
				return ss.TicksToMicroseconds(t + adjustment);
			},
			percentile
		);
		PushResult(resultName, mean);
	}

	template<typename TValue>
	static TValue ParseInteger(const std::wstring& string)
	{
		TValue dest;
		std::wstringstream temp;
		temp << string;
		temp >> dest;
		if(temp.fail())
		{
			throw std::exception("Not an integer.");
		}
		return dest;
	}

protected:
	struct Result
	{
	public:
		Result(std::wstring name, double value)
			: Name(name), Value(value) 
		{ }

	public:
		std::wstring Name;
		double Value;
	};

public:
	static const unsigned int CacheLineSize = 0x40; //TO DO: make sure this is the appropriate cache size (64 bytes)
	static const unsigned int MaxCacheSize = 0x800000;  //8MB TO DO: verify this as well/cite source
	
	static const uint32_t LocalMachineAddress = (127 << 0) | (0 << 8)  | (0 << 16) | (1 << 24);

	// other machine on LAN
	static const uint32_t OtherMachineAddress = (192 << 0) | (168 << 8)  | (1 << 16) | (65 << 24);
	// google
	//static const uint32_t OtherMachineAddress = (74 << 0) | (125 << 8)  | (224 << 16) | (176 << 24);


protected:
	StopwatchStack& _stopwatch;
	int _defaultNumIterations;

private:
	std::wstring _name;
	std::vector<Result> _results;
};
