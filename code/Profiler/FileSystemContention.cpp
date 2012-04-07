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


#include "FileSystemContention.h"
#include "Windows.h"
#undef CreateProcess
#include <sstream>

const std::wstring FileSystemContention::EventNameChildProcessesMayQuit = L"Local\\CSE221-FileSystemContention-ProcessesMayQuit";

FileSystemContention::FileSystemContention(StopwatchStack& ss)
	: FileSystemExperiment(L"File System Contention", ss)
{
	_defaultNumIterations = 2;
}

FileSystemContention::~FileSystemContention(void)
{

}

void FileSystemContention::Run(void)
{
	StopwatchStack& ss = _stopwatch;
	auto* tag = "FileSystem Contention";

	const int numProcesses = 8;
	const int fileSizeToUseMain = 1 << 26;
	const int fileSizeToUseChildren = 1 << 27;
	auto sectorSize = GetLogicalSectorSize();

	// prepare filename
	std::wstring filenameMain = GetTestFilenameBySize(fileSizeToUseMain);
	std::wstring filenameChildren = GetTestFilenameBySize(fileSizeToUseChildren);
	
	// prepare synchronization object
	HANDLE conditionQuit;
	{
		SECURITY_ATTRIBUTES securityAttributes;
		securityAttributes.bInheritHandle = TRUE;
		securityAttributes.lpSecurityDescriptor = NULL;
		securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		conditionQuit = CreateEvent(&securityAttributes, TRUE, FALSE, EventNameChildProcessesMayQuit.c_str());
	}

	// create processes
	std::vector<HANDLE> processes;
	{
		// create a number of threads.
		for(int i = 0; i < numProcesses; ++i)
		{
			auto* processHandle = FileSystemContention::CreateProcess(i, filenameChildren);
			processes.push_back(static_cast<HANDLE>(processHandle));
		}
	}


	// rest a moment
	Sleep(5 * 1000);

	// measure performance
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		void* fileHandle = OpenFileNoBuffering(filenameMain);
		ReadEntireFile(fileHandle, sectorSize, ss, tag, 0, RANDOM);
		CloseFile(fileHandle);
	}

	// signal child processes to quit.
	SetEvent(conditionQuit);

	// wait for child processes to quit.
	WaitForMultipleObjects(
		numProcesses,
		&(processes[0]),
		TRUE, 
		INFINITE
	);

	// store results
	PushResultMedianAsMicroseconds(
		L"Sequential Read Median (μs)",
		ss.GetTagHistory(tag),
		-ss.GetOverheadSimpleAsTicks()
	);

	PushResultPercentileAsMicroseconds(
		L"Sequential Read 10th Percentile (μs)",
		ss.GetTagHistory(tag),
		10,
		-ss.GetOverheadSimpleAsTicks()
	);

	PushResultPercentileAsMicroseconds(
		L"Sequential Read 90th Percentile (μs)",
		ss.GetTagHistory(tag),
		90,
		-ss.GetOverheadSimpleAsTicks()
	);
}

void* FileSystemContention::CreateProcess(int localProcessId, const std::wstring& fileToRead)
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
		<< L" -FileSystemChildProcess "
		<< L" "
		<< localProcessId
		<< L" \""
		<< fileToRead
		<< L"\""
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

void FileSystemContention::ChildProcessMain(int argc, wchar_t* argv[])
{
	if(argc < 4)
	{
		throw std::exception("Missing arguments.");
	}

	// parse arguments
	int localProcessId = ParseInteger<int>(argv[2]);
	std::wstring fileToRead = argv[3];

	// open file
	HANDLE hFile = (HANDLE)OpenFileNoBuffering(fileToRead);

	// get file info
	uint64_t fileSize = GetFileSize(hFile);

	// decide on start position
	uint32_t startPosition = static_cast<uint32_t>(fileSize / 10 * localProcessId);
	
	// open synchronization event
	HANDLE conditionMayQuit = OpenEvent(SYNCHRONIZE, FALSE, EventNameChildProcessesMayQuit.c_str());
	if(conditionMayQuit == NULL)
	{
		DWORD errorCode = GetLastError();
		throw std::exception("Error opening condition variable.");
	}

	StopwatchStack stopwatchDummy;

	// read and read
	while(WaitForSingleObject(conditionMayQuit, 0) == WAIT_TIMEOUT)
	{
		// read
		ReadEntireFile(hFile, DefaultBufferSize, stopwatchDummy, 0, 0, RANDOM);
	}

	// cleanup
	CloseFile(hFile);
}
