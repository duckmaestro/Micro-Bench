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


#include "SystemCallOverhead.h"
#include "Windows.h"

SystemCallOverhead::SystemCallOverhead(StopwatchStack& stopwatch)
	: Experiment(L"System Call Overhead", stopwatch)
{
}

SystemCallOverhead::~SystemCallOverhead(void)
{
}

void SystemCallOverhead::Run(void)
{
	StopwatchStack& ss = _stopwatch;
	auto tagSystemCallOverhead = L"SystemCallOverhead";

	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		// using invalid handle so that the kernel call won't actually do anything.

		ss.Push(tagSystemCallOverhead);
		SetThreadAffinityMask(INVALID_HANDLE_VALUE, 0);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1); // yield early to reduce our likelihood of being switched during timing.
		}
	}

	PushResultMeanAsMicroseconds(
		L"System Call Overhead (μs)", 
		ss.GetTagHistory(tagSystemCallOverhead),
		-ss.GetOverheadSimpleAsTicks()
	);
}