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


#pragma once

#include <map>
#include <cstdint>
#include "Experiment.h"

class FakeKernel : public Experiment
{
public:
	FakeKernel(StopwatchStack& stopwatch);
	virtual ~FakeKernel(void);
	virtual void Run(void);

protected:
	class FakeKernelObject 
	{
	public:
		char data[512];
	};

protected:
	void FakeValidateUserArguments(uint32_t handle, uint32_t threadId, uint32_t processId, uint64_t registryKeyId, uint64_t socketId);
	void FakeResourceAllocation(int memorySize, char keySize);

protected:
	uint32_t NumInitialHandles;
	uint32_t NumInitialThreads;
	uint32_t NumInitialProcesses;
	uint32_t NumInitialRegistryKeys;
	uint32_t NumInitialSockets;

protected:
	std::map<uint32_t, FakeKernelObject> _handleObjects;
	std::map<uint32_t, FakeKernelObject> _threads;
	std::map<uint32_t, FakeKernelObject> _processes;
	std::map<uint64_t, FakeKernelObject> _registryKeys;
	std::map<uint64_t, FakeKernelObject> _sockets;
};

