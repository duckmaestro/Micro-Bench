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


#include "TaskCreation.h"
#include "Windows.h"

DWORD WINAPI TaskCreation_ThreadMain(LPVOID argument);

TaskCreation::TaskCreation(StopwatchStack& stopwatch)
	: Experiment(L"Task Creation Time", stopwatch)
{
}

TaskCreation::~TaskCreation(void)
{
}

void TaskCreation::Run(void)
{
	StopwatchStack& ss = _stopwatch;
	auto tagCreateThread = L"CreateThread";
	auto tagCreateProcess = L"CreateProcess";

	// CreateThread(...)
	{
		for(int i = 0; i < 1000; ++i)
		{
			HRESULT errorCode;

			ss.Push(tagCreateThread);
			HANDLE hThread = CreateThread(NULL, 0, &TaskCreation_ThreadMain, NULL, 0, NULL);
			if(hThread == NULL)
			{
				errorCode = GetLastError();
				throw std::exception();
			}
			WaitForSingleObject(hThread, INFINITE); // wait for thread to run and exit.
			ss.Pop();

			// do not measure thread cleanup time.
			if(!CloseHandle(hThread))
			{
				errorCode = GetLastError();
				throw std::exception();
			}
		}

		PushResultMedianAsMicroseconds(
			L"CreateThread (μs)",
			ss.GetTagHistory(tagCreateThread),
			-ss.GetOverheadSimpleAsTicks()
		);
	}

	// CreateProcess(...)
	{
		for(int i = 0; i < 1000; ++i)
		{
			HRESULT errorCode;

			STARTUPINFO processStartupInfo;
			ZeroMemory(&processStartupInfo, sizeof(STARTUPINFO));
			PROCESS_INFORMATION processInfo;
			ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
			processStartupInfo.cb = sizeof(STARTUPINFO);

			// http://superuser.com/questions/381103/is-there-a-windows-exe-that-does-nothing
			wchar_t commandLine[] = L"svchost.exe"; 

			ss.Push(tagCreateProcess);
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
			}
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			ss.Pop();

			if(!CloseHandle(processInfo.hProcess))
			{
				errorCode = GetLastError();
				throw std::exception();
			}
		}

		PushResultMedianAsMicroseconds(
			L"CreateProcess (μs)",
			ss.GetTagHistory(tagCreateProcess),
			-ss.GetOverheadSimpleAsTicks()
		);
	}

}

DWORD WINAPI TaskCreation_ThreadMain(LPVOID argument)
{
	return 0;
}