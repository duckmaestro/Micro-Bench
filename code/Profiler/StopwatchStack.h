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

#ifndef DISABLE_WIN32_PERFORMANCE_TIMER
#define DISABLE_WIN32_PERFORMANCE_TIMER 1
#endif

#include <cstdint>
#include <vector>
#include <map>
#include <deque>
#if !DISABLE_WIN32_PERFORMANCE_TIMER
#include "Windows.h"
#endif

typedef int64_t TickValue;

class StopwatchStack
{
public:
	StopwatchStack(int reservationSize = 100, int tagHistorySize = 100, bool useWin32Kernel = false);
	virtual ~StopwatchStack(void);
private: 
	StopwatchStack& operator=(const StopwatchStack&);
	StopwatchStack(const StopwatchStack&);

public:
	__forceinline void Push(void);
	__forceinline void Push(const void* tag);
	void PushWithValue(const void* tag, TickValue value);
	__forceinline TickValue Pop(void);
	TickValue PopWithValue(const void* tag, TickValue value);
	__forceinline TickValue Read(void);

	double TicksToSeconds(TickValue ticks) const;
	double TicksToMilliseconds(TickValue ticks) const;
	double TicksToMicroseconds(TickValue ticks) const;
	TickValue MillisecondsToTicks(double milliseconds) const;

	void CalculateOverheadSimple();
	void CalculateOverheadNested();
	TickValue GetOverheadSimpleAsTicks() const
	{
		return _overheadSimple;
	}
	double GetOverheadSimpleAsMicroSeconds() const
	{
		return TicksToMicroseconds(_overheadSimple);
	}

	typedef std::deque<TickValue> TagHistory;
	TagHistory& GetTagHistory(const void* tag);
	void ResetTagHistory(const void* tag);

private:
	__forceinline void ReadHighresolutionTimer(TickValue& result);
	void StoreResultWithTag(TickValue result, const void* tag);

private:
	struct StopwatchEntry
	{
	public:
		TickValue startTime;
		const void* tag;
	};

private:
	std::vector<StopwatchEntry>::size_type _maxDepth;
	std::vector<StopwatchEntry> _stack;
	
	TagHistory::size_type _tagHistorySize;
	std::map<const void*, TagHistory*> _tagHistory;

	bool _useWin32Kernel;
	TickValue _ticksPerSecond;
	double _ticksPerSecondFP;
	double _ticksPerMillisecondFP;
	double _ticksPerMicrosecondFP;

	TickValue _overheadSimple;
	TickValue _overheadNested;

	void* _criticalSection;
};

#include "StopwatchStack.inl"
