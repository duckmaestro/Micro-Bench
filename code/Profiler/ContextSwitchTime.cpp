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


#include "ContextSwitchTime.h"
#include "Windows.h"
#undef CreateProcess
#include <cassert>
#include <string>
#include <vector>
#include <sstream>

DWORD WINAPI ContextSwitchTime_ThreadMain(void* arg);

ContextSwitchTime::ContextSwitchTime(StopwatchStack& stopwatch)
	: Experiment(L"Context Switch Time", stopwatch)
	, _criticalSectionStopwatch(0)
	, _conditionThreadsMayStart(0)
	, _threadSwitchStopwatchTag(L"ThreadContextSwitch")
	, _processSwitchStopwatchTag(L"ProcessContextSwitch")
	, _threadIdCounter(0)
{
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSectionEx(cs, 0, CRITICAL_SECTION_NO_DEBUG_INFO);
	_criticalSectionStopwatch = cs;
}

ContextSwitchTime::~ContextSwitchTime(void)
{
}

void ContextSwitchTime::Run(void)
{
	RunUsingThreads();
	RunUsingProcesses();
}

void ContextSwitchTime::RunUsingThreads(void)
{
	StopwatchStack& ss = _stopwatch;

	assert(_conditionThreadsMayStart == 0);
	_conditionThreadsMayStart = CreateEvent(NULL, TRUE, FALSE, L"Local\\CSE221-ContextSwitchTime-ThreadsMayStart");

	// setup
	int threadWaitBatchSize = 64; //MAXIMUM_WAIT_OBJECTS
	int threadCount = threadWaitBatchSize * 10;
	std::vector<HANDLE> threads;
	{
		// create a number of threads.
		for(int i = 0; i < threadCount; ++i)
		{
			auto newThread = CreateThread();
			threads.push_back(static_cast<HANDLE>(newThread));
		}
	}

	// shared memory
	_threadSharedMemory = malloc(1024 * 1024);

	// action
	TickValue startTick;
	TickValue endTick;
	{
		// sleep the main thread a moment
		Sleep(1000);

		// read start time
		startTick = ss.Read();	
		// open the flood gates
		SetEvent(_conditionThreadsMayStart);
		// wait for all threads
		for(int i = 0; i < threadCount / threadWaitBatchSize; ++i)
		{
			WaitForMultipleObjects(
				threadWaitBatchSize, 
				&(threads[i * threadWaitBatchSize]), 
				TRUE, 
				INFINITE
			);
		}
		// read last time
		endTick = ss.Read();
	}

	// reorganized data
	{
		// store first and last time in shared memory structure
		TickValue* pSharedMemoryAsTicks = static_cast<TickValue*>(_threadSharedMemory);
		pSharedMemoryAsTicks[0] = startTick;
		pSharedMemoryAsTicks[threadCount + 1] = endTick;

		// sort the data
		std::vector<TickValue> dataCollected;
		for(int i=0; i<threadCount + 2; ++i)
		{
			dataCollected.push_back(pSharedMemoryAsTicks[i]); // this is slow but whatever.
		}
		std::sort(dataCollected.begin(), dataCollected.end());

		// store the results in the stopwatch
		ss.PushWithValue(_threadSwitchStopwatchTag, dataCollected[0]);
		for(unsigned int i = 1; i < dataCollected.size() - 1; ++i)
		{
			ss.PopWithValue(_threadSwitchStopwatchTag, dataCollected[i]);
			ss.PushWithValue(_threadSwitchStopwatchTag, dataCollected[i]);
		}
		ss.PopWithValue(_threadSwitchStopwatchTag, dataCollected[dataCollected.size()-1]);
	}

	// reporting
	{
		PushResultMedianAsMicroseconds(
			L"Thread Switch Time (μs)",
			ss.GetTagHistory(_threadSwitchStopwatchTag),
			-ss.GetOverheadSimpleAsTicks()
		);
	}

	// cleanup
	{
		for(auto iter = threads.begin(); iter != threads.end(); ++iter)
		{
			CloseHandle(*iter);
		}

		CloseHandle(_conditionThreadsMayStart);
		_conditionThreadsMayStart = 0;
		free(_threadSharedMemory);
	}
}

void ContextSwitchTime::RunUsingProcesses(void)
{
	StopwatchStack& ss = _stopwatch;

	// some reusable security attributes for permitting process inheritence
	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);


	assert(_conditionThreadsMayStart == 0);
	_conditionThreadsMayStart = CreateEvent(&securityAttributes, TRUE, FALSE, L"Local\\CSE221-ContextSwitchTime-ThreadsMayStart");

	// create shared memory
	HANDLE hSharedMemory = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		&securityAttributes,
		PAGE_READWRITE,
		0,
		1024 * 1024,
		L"Local\\CSE221-ContextSwitchTime-ProcessSharedData"
	);
	if(hSharedMemory == NULL)
	{
		DWORD errorCode = GetLastError();
		throw std::exception("Error creating shared memory.");
	}

	void* pSharedMemory = MapViewOfFile(
		hSharedMemory,
		FILE_MAP_WRITE,
		0,
		0,
		1024 * 1024
	);
	if(pSharedMemory == NULL)
	{
		DWORD errorCode = GetLastError();
		throw std::exception("Error creating shared memory.");
	}

		
	// setup
	int processWaitBatchSize = 64; //MAXIMUM_WAIT_OBJECTS
	int processCount = processWaitBatchSize * 10;
	std::vector<HANDLE> processes;
	{
		// create a number of processes.
		for(int i = 0; i < processCount; ++i)
		{
			auto newThread = CreateProcess(i);
			processes.push_back(static_cast<HANDLE>(newThread));
		}
	}

	// action
	TickValue startTick;
	TickValue endTick;
	{
		// sleep the main thread a moment
		Sleep(1000);

		// read start time
		startTick = ss.Read();		
		// open the flood gates
		SetEvent(_conditionThreadsMayStart);
		// wait for all processes
		for(int i = 0; i < processCount / processWaitBatchSize; ++i)
		{
			WaitForMultipleObjects(
				processWaitBatchSize, 
				&(processes[i * processWaitBatchSize]), 
				TRUE, 
				INFINITE)
			;
		}
		// read last time
		endTick = ss.Read();
	}

	// reorganized data
	{
		// store first and last time in shared memory structure
		TickValue* pSharedMemoryAsTicks = static_cast<TickValue*>(pSharedMemory);
		pSharedMemoryAsTicks[0] = startTick;
		pSharedMemoryAsTicks[processCount + 1] = endTick;

		// sort the data
		std::vector<TickValue> dataCollected;
		for(int i=0; i<processCount + 2; ++i)
		{
			dataCollected.push_back(pSharedMemoryAsTicks[i]); // this is slow but whatever.
		}
		std::sort(dataCollected.begin(), dataCollected.end());

		// store the results in the stopwatch
		ss.PushWithValue(_processSwitchStopwatchTag, dataCollected[0]);
		for(unsigned int i = 1; i < dataCollected.size() - 1; ++i)
		{
			ss.PopWithValue(_processSwitchStopwatchTag, dataCollected[i]);
			ss.PushWithValue(_processSwitchStopwatchTag, dataCollected[i]);
		}
		ss.PopWithValue(_processSwitchStopwatchTag, dataCollected[dataCollected.size()-1]);
	}

	// reporting
	{
		PushResultMedianAsMicroseconds(
			L"Process Switch Time (μs)",
			ss.GetTagHistory(_processSwitchStopwatchTag),
			-ss.GetOverheadSimpleAsTicks()
		);
	}

	// cleanup
	{
		for(auto iter = processes.begin(); iter != processes.end(); ++iter)
		{
			CloseHandle(*iter);
		}

		CloseHandle(_conditionThreadsMayStart);
		_conditionThreadsMayStart = 0;

		UnmapViewOfFile(pSharedMemory);
		CloseHandle(hSharedMemory);
	}
}

void* ContextSwitchTime::CreateThread(void)
{
	HRESULT errorCode;
	HANDLE hThread = ::CreateThread(
		NULL, 
		0, 
		&ContextSwitchTime_ThreadMain, 
		this, 
		0, 
		NULL
	);
	
	if(hThread == NULL)
	{
		errorCode = GetLastError();
		throw std::exception();
	}
	
	SetThreadAffinityMask(hThread, 1 << 0);
	
	if(!SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL))
	{
		errorCode = GetLastError();
		throw std::exception();
	}

	return static_cast<void*>(hThread);
}

void* ContextSwitchTime::CreateProcess(int localProcessId)
{
	wchar_t path[1024];
	GetModuleFileName(NULL, path, 1024);
	
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

	std::wstringstream stringBuilder;
	stringBuilder 
		<< L"Profiler.exe"
		<< L" -ContextSwitchTimeChildProcess "
		<< localProcessId
	;

	BOOL success = CreateProcessW(
		path,
		const_cast<LPWSTR>(stringBuilder.str().c_str()),
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&startupInfo,
		&processInfo
	);

	if(!success)
	{
		throw std::exception("Could not create process.");
	}

	return processInfo.hProcess;
}

void ContextSwitchTime::ThreadMain()
{
	int localThreadId;
	{
		EnterCriticalSection(static_cast<LPCRITICAL_SECTION>(_criticalSectionStopwatch));
		localThreadId = _threadIdCounter++;
		LeaveCriticalSection(static_cast<LPCRITICAL_SECTION>(_criticalSectionStopwatch));
	}

	StopwatchStack& ss = _stopwatch;
	
	WaitForSingleObject(_conditionThreadsMayStart, INFINITE);
	
	// record the current time, then yield right away.
	TickValue ticks = ss.Read();
	Sleep(1000);

	// record current time
	TickValue* pSharedMemoryAsTicks = static_cast<TickValue*>(_threadSharedMemory);
	pSharedMemoryAsTicks[1 + localThreadId] = ticks; // 1 +, to save the first slot for main thread.
	
	return;
}

void ContextSwitchTime::ChildProcessMain(int argc, wchar_t* argv[])
{
	if(argc < 3)
	{
		throw std::exception("Missing arguments.");
	}

	//__asm int 3;

	int localProccessId = 0;
	{
		std::wstringstream arg3AsStream;
		arg3AsStream << std::wstring(argv[2]);
		arg3AsStream >> localProccessId;
	}	

	// setup a stopwatch
	StopwatchStack ss;

	// open shared memory
	HANDLE hSharedMemory;
	void* pSharedMemory;
	{
		hSharedMemory = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			1024 * 1024,
			L"Local\\CSE221-ContextSwitchTime-ProcessSharedData"
		);
		if(hSharedMemory == NULL)
		{
			DWORD errorCode = GetLastError();
			throw std::exception("Error creating shared memory.");
		}

		pSharedMemory = MapViewOfFile(
			hSharedMemory,
			FILE_MAP_WRITE,
			0,
			0,
			1024 * 1024
		);
		if(pSharedMemory == NULL)
		{
			DWORD errorCode = GetLastError();
			throw std::exception("Error creating shared memory.");
		}
	}

	// open shared condition variable
	HANDLE conditionThreadsMayStart = OpenEvent(SYNCHRONIZE, FALSE, L"Local\\CSE221-ContextSwitchTime-ThreadsMayStart");
	if(conditionThreadsMayStart == NULL)
	{
		DWORD errorCode = GetLastError();
		throw std::exception("Error opening condition variable.");
	}



	// wait at the starting line
	{
		WaitForSingleObject(conditionThreadsMayStart, INFINITE);
	}

	// record the current time, then yield right away.
	TickValue ticks = ss.Read();
	Sleep(1000);

	// record current time
	TickValue* pSharedMemoryAsTicks = static_cast<TickValue*>(pSharedMemory);
	pSharedMemoryAsTicks[1 + localProccessId] = ticks; // 1 +, to save the first slot for main thread.


	// cleanup
	{
		CloseHandle(conditionThreadsMayStart);
		UnmapViewOfFile(pSharedMemory);
		CloseHandle(hSharedMemory);
	}


}

DWORD WINAPI ContextSwitchTime_ThreadMain(void* param)
{
	ContextSwitchTime& This = *static_cast<ContextSwitchTime*>(param);
	This.ThreadMain();
	return 0;
}