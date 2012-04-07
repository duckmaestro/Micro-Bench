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


#include "LoopOverhead.h"
#include "Mathx.h"
#include "Windows.h"

// to prevent the compiler from reusing other methods, the test procedures depend on this private static variable.
static int g_hiddenValue;

LoopOverhead::LoopOverhead(StopwatchStack& stopwatch)
	: Experiment(L"Loop Overhead", stopwatch)
{

}

void LoopOverhead::Run()
{
	StopwatchStack& ss = _stopwatch;
	void* tag = L"CPU-Scheduling -- Loop Overhead per 1000 iterations";
	
	for(int i=0; i < _defaultNumIterations; i++){
		ss.Push(tag);
		for(int j=0; j<1000; j++){
		}
		ss.Pop();
	}
	PushResultMeanAsMicroseconds(
			L"Loop Overhead per 1000 iterations", 
			(ss.GetTagHistory(tag)),
			-ss.GetOverheadSimpleAsTicks()
		);
}