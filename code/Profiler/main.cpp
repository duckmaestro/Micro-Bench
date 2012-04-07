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


#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <string>
#include "Windows.h"
#include "StopwatchStack.h"
#include "Experiments.h"

int wmain(int argc, wchar_t* argv[]);
void configureConsoleForUnicode(bool=false);
void waitForEnterToQuit();
void nothingFancy();
void performExperiments();

int wmain(int argc, wchar_t* argv[])
{
	// fix unicode console
	{
		configureConsoleForUnicode();
	}

	if(argc < 2)
	{
		std::wcout << std::endl;
		std::wcout << L" ======================================= " << std::endl;
		std::wcout << L" __/*/*/*/__ Micro Bench __\\*\\*\\*\\__ " << std::endl;
		std::wcout << L" ======================================= " << std::endl << std::endl;
	}

	// process & thread config
	{
		// boost process priority
		HANDLE hProcess = GetCurrentProcess();
		SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);

		// lock thread to core 0
		HANDLE hThread = GetCurrentThread();
		SetThreadAffinityMask(hThread, 1 << 0);
			
		// boost main thread priority
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	}

	// do work
	{
		if(argc >= 2)  //this is a child process, deal with specially for part 1.4
		{
			if(std::wstring(argv[1]) == std::wstring(L"-ContextSwitchTimeChildProcess"))
			{
				ContextSwitchTime::ChildProcessMain(argc, argv);
				return 0;
			}
			else if(std::wstring(argv[1]) == std::wstring(L"-NetworkBandwidthChildServer"))
			{
				NetworkPeakBandwidth::ChildProcessMain(argc, argv);
				return 0;
			}
			else if(std::wstring(argv[1]) == std::wstring(L"-FileSystemChildProcess"))
			{
				FileSystemContention::ChildProcessMain(argc, argv);
				return 0;
			}

			throw std::exception("Unknown commandline arguments.");
		}
		else
		{
			//nothingFancy();
			performExperiments();
		}
	}

	// exit
	{
		waitForEnterToQuit();
		return 0;
	}
}

void performExperiments()
{
	// stopwatch
	StopwatchStack ss(100, 1000, false);
	std::wcout << L"Calculating Stopwatch overhead..." << std::endl;
	ss.CalculateOverheadSimple();

	// experiment list
	std::vector<std::auto_ptr<Experiment>> experiments;

	// Set 1
	if(0)
	{
		experiments.push_back(std::auto_ptr<Experiment>(new MeasurementOverhead(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new ProcedureOverhead(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new SystemCallOverhead(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new TaskCreation(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new ContextSwitchTime(ss)));
	}

	// Set 2
	if(0)
	{
		experiments.push_back(std::auto_ptr<Experiment>(new MemoryLatency(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new MemoryBandwidth(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new MemoryPageFault(ss)));
	}

	// Set 3
	if(0)
	{
		experiments.push_back(std::auto_ptr<Experiment>(new NetworkRoundTripTime(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new NetworkPeakBandwidth(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new NetworkSetupTeardown(ss)));
	}

	// Set 4
	if(0)
	{
		experiments.push_back(std::auto_ptr<Experiment>(new FileSystemCacheSize(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new FileSystemLocalRead(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new FileSystemRemoteRead(ss)));
		experiments.push_back(std::auto_ptr<Experiment>(new FileSystemContention(ss)));
	}

	// Set X
	if(0)
	{
		experiments.push_back(std::auto_ptr<Experiment>(new FakeKernel(ss)));
	}

	// perform experiments
	for(auto iter = experiments.begin(); iter != experiments.end(); ++iter)
	{
		Experiment* experiment = (*iter).get();
		std::wcout 
			<< L"Running '" 
			<< experiment->GetName() 
			<< L"'..."
			<< std::endl;
		experiment->Run();
	}

	// print results
	std::wcout << L"Printing results..." << std::endl;
	for(auto iter = experiments.begin(); iter != experiments.end(); ++iter)
	{
		Experiment* experiment = (*iter).get();
		std::wcout << experiment->ToString();
	}
}

void nothingFancy()
{
	// initialize a stopwatch stack
	StopwatchStack ss(100, 10, false);
	std::wcout << "StopwatchStack initialized." << std::endl;

	// start a watch
	std::wcout << "Pushing a stopwatch." << std::endl;
	ss.Push();

	std::wcout << L"Commencing message box..." << std::endl;
	MessageBox(HWND_DESKTOP, L"Testing Windows Message Box", L"blah", 0);
	std::wcout << L"Sleeping..." << std::endl;
	Sleep(1000);

	// stop the last watch
	std::wcout << L"Popping a stopwatch." << std::endl;
	uint64_t deltaTicks = ss.Pop();
	double deltaSeconds = ss.TicksToSeconds(deltaTicks);

	// output delta time.
	std::wcout 
		<< L"Stopwatch delta: \t"
		<< deltaTicks << L" ticks. "
		<< deltaSeconds << L" seconds." 
		<< std::endl;
}

void waitForEnterToQuit()
{
	// pause & quit.
	std::wcout << L"Press enter to quit." << std::endl;
	{
		wchar_t in;
		std::wcin.read(&in, 1);
	}
}

void configureConsoleForUnicode(bool test)
{
	// http://stackoverflow.com/questions/2492077/output-unicode-strings-in-windows-console-app
	_setmode(_fileno(stdout), _O_U16TEXT);

	if(test)
	{
		std::wcout << L"Testing unicode -- English -- Ελληνικά -- Español." << std::endl;
	}
}