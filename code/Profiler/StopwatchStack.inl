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


__forceinline void StopwatchStack::Push(void)
{
	Push(0);
}

__forceinline void StopwatchStack::Push(const void* tag)
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

	// read time as late as possible
	ReadHighresolutionTimer(entry.startTime);
	
	return;
}

__forceinline TickValue StopwatchStack::Pop(void)
{
	// read time as soon as possible
	TickValue timerValue;
	ReadHighresolutionTimer(timerValue);

	if(_stack.empty())
	{
		return 0;
	}
	
	// grab most recent entry
	StopwatchEntry entry = _stack.back();
	_stack.pop_back();
	
	// calculate & return delta
	TickValue delta = timerValue - entry.startTime;

	// store in history?
	if(entry.tag != 0)
	{
		StoreResultWithTag(delta, entry.tag);
	}

	return delta;
}

__forceinline TickValue StopwatchStack::Read(void)
{
	TickValue timerValue;
	ReadHighresolutionTimer(timerValue);
	return timerValue;
}

__forceinline void StopwatchStack::ReadHighresolutionTimer(TickValue& result)
{		
	// http://stackoverflow.com/questions/3388134/rdtsc-accuracy-across-cpu-cores/4145156#4145156
	// http://www.ccsl.carleton.ca/~jamuir/rdtscpm1.pdf


#if !DISABLE_WIN32_PERFORMANCE_TIMER
	if(_useWin32Kernel)
	{
		LARGE_INTEGER value;
		QueryPerformanceCounter(&value);
		result = value.QuadPart;
	}
	else
#endif
	{
		__asm
		{
			push eax
			push ebx
			push ecx
			push edx
			
			mov eax, 0
			// http://stackoverflow.com/questions/2918113/cpuid-before-rdtsc
			cpuid
			mov ecx, result
			rdtsc
			mov [DWORD PTR ecx + 0], eax
			mov [DWORD PTR ecx + 4], edx

			// would be nice if i didn't have to copy the pointer to ecx (!....)
			//mov [DWORD PTR resultAsPointer + 0], eax
			//mov [DWORD PTR resultAsPointer + 4], edx

			pop edx
			pop ecx
			pop ebx
			pop eax
		};
		return;
	}
}
