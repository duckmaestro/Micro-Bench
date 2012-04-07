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


#include "FakeKernel.h"
#include <cassert>

FakeKernel::FakeKernel(StopwatchStack& stopwatch)
	: Experiment(L"Fake Kernel", stopwatch)
	, NumInitialHandles(23 * 1024)
	, NumInitialThreads(1 * 1024)
	, NumInitialProcesses(80)
	, NumInitialRegistryKeys(100 * 1024)
	, NumInitialSockets(128)
{
	for(uint32_t i = 1; i <= NumInitialHandles; ++i)
	{
		_handleObjects.insert(
			std::pair<uint32_t, FakeKernelObject>(i, FakeKernelObject())
		);
	}

	for(uint32_t i = 1; i <= NumInitialThreads; ++i)
	{
		_threads.insert(
			std::pair<uint32_t, FakeKernelObject>(i, FakeKernelObject())
		);
	}

	for(uint32_t i = 1; i <= NumInitialProcesses; ++i)
	{
		_processes.insert(
			std::pair<uint32_t, FakeKernelObject>(i, FakeKernelObject())
		);
	}

	for(uint32_t i = 1; i <= NumInitialRegistryKeys; ++i)
	{
		_registryKeys.insert(
			std::pair<uint64_t, FakeKernelObject>(i, FakeKernelObject())
		);
	}

	for(uint32_t i = 1; i <= NumInitialSockets; ++i)
	{
		_sockets.insert(
			std::pair<uint64_t, FakeKernelObject>(i, FakeKernelObject())
		);
	}
}

FakeKernel::~FakeKernel(void)
{
	_handleObjects.clear();
	_threads.clear();
	_processes.clear();
	_registryKeys.clear();
	_sockets.clear();
}

void FakeKernel::Run(void)
{
	StopwatchStack& ss = _stopwatch;

	{
		auto tag = L"Verify User Arguments";

		for(int i = 0; i < _defaultNumIterations; ++i)
		{
			uint32_t handle = rand() % _handleObjects.size() + 1;
			uint32_t thread = rand() % _threads.size() + 1;
			uint32_t process = rand() % _processes.size() + 1;
			uint64_t registryKey = rand() % _registryKeys.size() + 1;
			uint64_t socket = rand() % _sockets.size() + 1;

			ss.Push(tag);
			FakeValidateUserArguments(handle, thread, process, registryKey, socket);
			ss.Pop();
		}

		PushResultMedianAsMicroseconds(
			L"User Argument Verification (μs)",
			ss.GetTagHistory(tag),
			-ss.GetOverheadSimpleAsTicks()
		);
	}


	{
		auto tag = L"Resource Allocation";

		for(int i = 0; i < _defaultNumIterations; ++i)
		{
			ss.Push(tag);
			FakeResourceAllocation(
				16 * 1024 * (i % 4 + 1), 
				4 * (i % 2 + 1)
			);
			ss.Pop();
		}

		PushResultMedianAsMicroseconds(
			L"Resource Allocation (μs)",
			ss.GetTagHistory(tag),
			-ss.GetOverheadSimpleAsTicks()
		);
	}
}

void FakeKernel::FakeValidateUserArguments(uint32_t handle, uint32_t threadId, uint32_t processId, uint64_t registryKeyId, uint64_t socketId)
{
	FakeKernelObject* object = NULL;
	FakeKernelObject* thread = NULL;
	FakeKernelObject* process = NULL;
	FakeKernelObject* registryKey = NULL;
	FakeKernelObject* socket = NULL;

	// locate objects
	{
		if(handle != 0)
		{
			auto iter = _handleObjects.find(handle);
			if(iter != _handleObjects.end())
			{
				object = &(iter->second);
			}
		}

		if(threadId != 0)
		{
			auto iter = _threads.find(threadId);
			if(iter != _threads.end())
			{
				thread = &(iter->second);
			}
		}

		if(processId != 0)
		{
			auto iter = _processes.find(processId);
			if(iter != _processes.end())
			{
				process = &(iter->second);
			}
		}

		if(registryKeyId != 0)
		{
			auto iter = _registryKeys.find(registryKeyId);
			if(iter != _registryKeys.end())
			{
				registryKey = &(iter->second);
			}
		}

		if(socketId != 0)
		{
			auto iter = _sockets.find(socketId);
			if(iter != _sockets.end())
			{
				socket = &(iter->second);
			}
		}
	}

	// inspect data
	FakeKernelObject* objects[5] 
		= { object, thread, process, registryKey, socket }
	;

	for(int i = 0; i < 5; ++i)
	{	
		FakeKernelObject* objA = objects[i];

		if(objA == NULL)
		{
			continue;
		}

		for(int j = 0; j < i; ++j)
		{
			FakeKernelObject* objB = objects[j];

			if(objB == NULL)
			{
				continue;
			}

			for(int r = 0; r < 48; ++r)
			{
				int r0 = rand() % sizeof(objA->data);
				int r1 = rand() % sizeof(objB->data);

				if(objA->data[r0] == objB->data[r1])
				{
					continue;
				}
			}
		}
	}
}

void FakeKernel::FakeResourceAllocation(int memorySize, char keySize)
{
	assert(keySize == 4 || keySize == 8);

	if(keySize == 4)
	{
		_handleObjects.insert(
			std::pair<uint32_t, FakeKernelObject>(-1, FakeKernelObject())
		);
	}
	else if(keySize == 8)
	{
		_registryKeys.insert(
			std::pair<uint64_t, FakeKernelObject>(-1, FakeKernelObject())
		);
	}
	void* mem = std::malloc(memorySize);
	if(mem == NULL)
	{
		throw std::exception();
	}

	std::free(mem);
	_handleObjects.erase(-1);
	_registryKeys.erase(-1);
}