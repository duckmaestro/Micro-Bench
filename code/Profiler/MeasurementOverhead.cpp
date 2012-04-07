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


#include "MeasurementOverhead.h"
#include "Mathx.h"
#include <vector>

MeasurementOverhead::MeasurementOverhead(StopwatchStack& stopwatch)
	: Experiment(L"Measurement Overhead", stopwatch)
{

}

void MeasurementOverhead::Run()
{
	StopwatchStack& ss = _stopwatch;
	
	// has sampling overhead been calculated yet?
	TickValue overhead = ss.GetOverheadSimpleAsTicks();
	if(overhead == 0)
	{
		ss.CalculateOverheadSimple();
	}

	// push result
	PushResult(L"Minimum (μs)", ss.GetOverheadSimpleAsMicroSeconds());

	return;
}