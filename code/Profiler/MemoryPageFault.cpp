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

#include "MemoryPageFault.h"
#include "Mathx.h"
#include "Windows.h"
#include <cstdlib>



MemoryPageFault::MemoryPageFault(StopwatchStack& stopwatch)
	: Experiment(L"Memory Page Fault Overhead", stopwatch)
{

}

void MemoryPageFault::Run()
{	
	
	StopwatchStack& ss = _stopwatch;
	const int numIterations = 100;
	void* tag = L"Memory -- Page Faults";
	const int ramNeeded = 16;   //try to malloc 16gb of RAM (way more than we have memory for)


	// reduce process priority
	HANDLE hProcess = GetCurrentProcess();
	SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

	// lock thread to core 0
	HANDLE hThread = GetCurrentThread();
	SetThreadAffinityMask(hThread, 0);
			
	// reduce main thread priority
	SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);

	/*memory page fault testin'
	*  info: http://support.microsoft.com/kb/2267427
	*/
	const int pageSize = 1 << 12;  //4KB
	const unsigned int oneGibby = 1 << 30;  //1GB
	HANDLE processes[ramNeeded];  //array to hold handles to all processes
	//initialize OUR GB of memory so that we measure page faults rather than "first-time misses"
	void *p = malloc(oneGibby);
	char *explorer = (char*) p;
	char *base = (char*) p;
	int pageNum = 0;  
	for(pageNum; pageNum < (oneGibby / pageSize); pageNum++)
	{
		explorer = base + ((pageNum * pageSize) % oneGibby);
		*explorer = pageNum % 256;
	}

	// launch the mem suckers
	for(int i=0; i < ramNeeded;i++)
	{
		HRESULT errorCode;
		STARTUPINFO processStartupInfo;
		ZeroMemory(&processStartupInfo, sizeof(STARTUPINFO));
		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
		processStartupInfo.cb = sizeof(STARTUPINFO);
		//adjust this
		wchar_t commandLine[] = L"memSucker.exe"; 
		if(!CreateProcess(
			NULL, 
			commandLine, 
			NULL,
			NULL,
			FALSE,
			NORMAL_PRIORITY_CLASS,
			NULL,
			NULL,
			&processStartupInfo,
			&processInfo
		))
		{
			errorCode = GetLastError();
			throw std::exception();
			return;
		}
		processes[i]=processInfo.hProcess;
	}
	//go to sleep to give these processes a chance to consume memory
	Sleep(10 * 1000);  //10 seconds?
	//now that we've created a poisonous environment with 16 gb+ of memory usage, let's test
	
	for(int r = 0; r < numIterations; ++r)
	{
		for(int pageNum = 0; pageNum < 1; pageNum++)
		{
			TickValue tv0 = ss.Read();
			explorer = base + ((pageNum * pageSize * r) % oneGibby); 
			TickValue tv1 = ss.Read();
			ss.PushWithValue(tag, tv0);
			ss.PopWithValue(tag, tv1);

			Sleep(100);
		}
	}

	free(p);

	//close out the other processes by terminating
	for(int i = 0; i < ramNeeded; i++) {

		if(!TerminateProcess(processes[i], 0))
		{
			HRESULT errorCode = GetLastError();
			//throw std::exception();
		}
	}

	PushResultMedianAsMicroseconds(
		L"Page Fault Time Median (μs)", 
		(ss.GetTagHistory(tag)),
		-ss.GetOverheadSimpleAsTicks()
	);

	PushResultMaximumAsMicroseconds(
		L"Page Fault Time Max (μs)", 
		(ss.GetTagHistory(tag)),
		-ss.GetOverheadSimpleAsTicks()
	);

}
