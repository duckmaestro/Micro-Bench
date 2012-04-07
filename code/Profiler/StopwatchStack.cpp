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


#include "StopwatchStack.h"
#include "Windows.h"
#include <cassert>
#include "Mathx.h"

StopwatchStack::StopwatchStack(int reservationSize, int tagHistorySize, bool useWin32Kernel)
	: _overheadSimple(0)
	, _overheadNested(0)
	, _criticalSection(0)
{

#if DISABLE_WIN32_PERFORMANCE_TIMER
	if(useWin32Kernel)
	{
		assert(!"Win32 timer not enabled in build.");
	}
#endif

	// save options
	_maxDepth = reservationSize;
	_useWin32Kernel = useWin32Kernel;
	_tagHistorySize = tagHistorySize;

	// reserve memory
	_stack.reserve(reservationSize);

	// initialize ticks per second
	{
		if(useWin32Kernel)
		{
			LARGE_INTEGER freq;
			QueryPerformanceFrequency(&freq);
			_ticksPerSecond = freq.QuadPart;
		}
		else
		{
			// x86 rdtsc uses "cycles". we need to ask windows what the clock rate is for the cpu.

			DWORD mhz;
			DWORD mhzSize = sizeof(mhz);
			RegGetValue(
				HKEY_LOCAL_MACHINE, 
				L"Hardware\\DESCRIPTION\\System\\CentralProcessor\\0",
				L"~Mhz",
				RRF_RT_REG_DWORD,
				NULL,
				&mhz,
				&mhzSize
			);

			_ticksPerSecond = mhz * 1000 * 1000;
		}

		_ticksPerSecondFP = (double)_ticksPerSecond;
		_ticksPerMillisecondFP = _ticksPerSecondFP / 1000.0;
		_ticksPerMicrosecondFP = _ticksPerSecondFP / (1000.0 * 1000.0);
	}

	// initialize critical section
	// (not used yet).
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(cs);
	_criticalSection = cs;
}

StopwatchStack::~StopwatchStack(void)
{
	free(_criticalSection);
	_criticalSection = 0;

	for(auto iter = _tagHistory.begin(); iter != _tagHistory.end(); ++iter)
	{
		delete (iter->second);
	}
	_tagHistory.clear();
}

double StopwatchStack::TicksToSeconds(TickValue ticks) const
{
	double ticksAsDouble = (double)ticks;
	double seconds = ticksAsDouble / _ticksPerSecondFP;
	return seconds;
}

double StopwatchStack::TicksToMilliseconds(TickValue ticks) const
{
	double ticksAsDouble = (double)ticks;
	double milliseconds = ticksAsDouble / _ticksPerMillisecondFP;
	return milliseconds;
}

double StopwatchStack::TicksToMicroseconds(TickValue ticks) const
{
	double ticksAsDouble = (double)ticks;
	double microseconds = ticksAsDouble / _ticksPerMicrosecondFP;
	return microseconds;
}

TickValue StopwatchStack::MillisecondsToTicks(double milliseconds) const
{
	double ticks = milliseconds * _ticksPerMillisecondFP;
	return static_cast<TickValue>(ticks);
}

void StopwatchStack::PushWithValue(const void* tag, TickValue value)
{
	if(_maxDepth <= _stack.size())
	{
		// output warning?
		return;
	}

	// prepare storage in the stack first.
	_stack.push_back(StopwatchEntry());
	StopwatchEntry& entry = _stack.back();

	// copy some values
	entry.tag = tag;

	// store value
	entry.startTime = value;
	
	return;
}

TickValue StopwatchStack::PopWithValue(const void* tag, TickValue value)
{
	if(_stack.empty())
	{
		return 0;
	}
	
	// grab most recent entry
	StopwatchEntry entry = _stack.back();
	_stack.pop_back();

	assert(entry.tag == tag);
	
	// calculate & return delta
	TickValue delta = value - entry.startTime;

	// store in history?
	if(entry.tag != 0)
	{
		StoreResultWithTag(delta, entry.tag);
	}

	return delta;
}

void StopwatchStack::StoreResultWithTag(TickValue result, const void* tag)
{
	if(_tagHistorySize == 0)
	{
		return;
	}
	auto& historyOfTagAsIter = _tagHistory.find(const_cast<void*>(tag));
	if(historyOfTagAsIter == _tagHistory.end())
	{
		_tagHistory.insert(
			std::pair<const void*, TagHistory*>(
				tag, 
				new TagHistory()
			)
		);
		historyOfTagAsIter = _tagHistory.find(const_cast<void*>(tag));
	}

	auto& historyOfTag = historyOfTagAsIter->second;
	if(historyOfTag->size() >= _tagHistorySize)
	{
		historyOfTag->pop_front();
	}

	historyOfTag->push_back(result);
}

StopwatchStack::TagHistory& StopwatchStack::GetTagHistory(const void* tag)
{
	static TagHistory empty;

	auto& historyAsIter = _tagHistory.find(const_cast<void*>(tag));
	if(historyAsIter == _tagHistory.end())
	{
		return empty;
	}
	else
	{
		return *(historyAsIter->second);
	}
}

void StopwatchStack::ResetTagHistory(const void* tag)
{
	auto& historyAsIter = _tagHistory.find(const_cast<void*>(tag));
	if(historyAsIter == _tagHistory.end())
	{
		return;
	}
	delete historyAsIter->second;
	_tagHistory.erase(historyAsIter);
}

void StopwatchStack::CalculateOverheadSimple()
{
	StopwatchStack& stopwatch = *this;
	int numQueries = 50000;
	wchar_t* tag = L"Measurement Overhead";

	for(int i=0; i < numQueries; ++i)
	{
		TickValue delta;
		stopwatch.Push(tag);
		delta = stopwatch.Pop();
	}

	// my back of the hand calculation 
	// is based solely on time between
	// back-to-back timer readings.

	TickValue median
		= Mathx::Minimum<TickValue, TickValue>(
			stopwatch.GetTagHistory(tag),
			[&stopwatch](TickValue t) 
			{ 
				return t;
			}
		);

	_overheadSimple = median;
}

void StopwatchStack::CalculateOverheadNested()
{
	assert(!"Not implemented.");
}
