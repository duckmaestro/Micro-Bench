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


#include "FileSystemRemoteRead.h"
#include "Windows.h"
#include <sstream>
#include <cassert>

FileSystemRemoteRead::FileSystemRemoteRead(StopwatchStack& ss)
	: FileSystemExperiment(L"File System Remote Reads", ss)
{
	_defaultNumIterations = 2;
	_devicePath = L"K:\\";
}

FileSystemRemoteRead::~FileSystemRemoteRead(void)
{
}

void FileSystemRemoteRead::Run(void)
{
	StopwatchStack& ss = _stopwatch;
	auto* baseTag = L"FileSystem -- Remote Unbuffered Read Latency Synchronous";

	uint32_t sectorSize = -1;
	uint32_t blockSize;
	uint32_t sectorsPerBlock = 1;

	sectorSize = FileSystemExperiment::GetLogicalSectorSize(_devicePath);
	assert(sectorSize > 1);
	blockSize = sectorSize*sectorsPerBlock;

	void* synchInputBuffer = std::malloc(sectorSize);
	assert(synchInputBuffer != NULL);

	uint64_t numFileSizes = _fileSizes.size();

	// reset tags
	for(uint32_t i = 0; i < numFileSizes; ++i)
	{
		auto tag = baseTag + i;
		ss.ResetTagHistory(tag);
	}

	// result names
	std::vector<std::wstring> resultNamesSequentialMedian;
	std::vector<std::wstring> resultNamesSequentialPercentile;
	std::vector<std::wstring> resultNamesRandomMedian;
	std::vector<std::wstring> resultNamesRandomPercentile;
	for (uint32_t i = 0; i < numFileSizes; i++)
	{
		uint64_t fileSize = _fileSizes.at(i);
		{
			std::wstringstream temp;
			temp << L"Sequential (" << fileSize << L" bytes) Median (μs)";
			resultNamesSequentialMedian.push_back(temp.str());
		}
		{
			std::wstringstream temp;
			temp << L"Sequential (" << fileSize << L" bytes) 90th Percentile (μs)";
			resultNamesSequentialPercentile.push_back(temp.str());
		}
		{
			std::wstringstream temp;
			temp << L"Random (" << fileSize << L" bytes) Median (μs)";
			resultNamesRandomMedian.push_back(temp.str());
		}
		{
			std::wstringstream temp;
			temp << L"Random (" << fileSize << L" bytes) 90th Percentile (μs)";
			resultNamesRandomPercentile.push_back(temp.str());
		}
	}

	for(int j = 0; j < _defaultNumIterations; ++j)
	{
		for(int t = 0; t < 2; ++t)
		{
			ReadMode readMode = t == 0 ? SEQUENTIAL : RANDOM;

			for (uint32_t i = 0; i < numFileSizes; i++)
			{
				auto tag = baseTag + i + (1000 * t);
				uint64_t currentFileSize = 1 << i;
				uint64_t flusherFileSize = 1 << 26;
		
				// attempt to flush any caches
				{
					if(flusherFileSize == currentFileSize)
					{
						flusherFileSize = flusherFileSize >> 1;
					}
					ReadEntireFile(GetTestFilenameBySize(flusherFileSize));
				}
						
				// prepare target filename
				std::wstring filename = GetTestFilenameBySize(currentFileSize);
		
				// open file
				HANDLE fileHandle = (HANDLE)FileSystemExperiment::OpenFileNoBuffering(filename);
				if(fileHandle == INVALID_HANDLE_VALUE)
				{
					HRESULT errorCode = GetLastError();
					throw std::exception();
				}

				// read 
				ReadEntireFile(fileHandle, DefaultBufferSize, ss, tag, 0, readMode);

				// close file
				FileSystemExperiment::CloseFile(fileHandle);
			}
		}
	}

	// push results
	for(uint32_t i = 0; i < numFileSizes; ++i)
	{
		auto tag = baseTag + i;
		
		PushResultMedianAsMicroseconds(
			resultNamesSequentialMedian[i], 
			ss.GetTagHistory(tag),
			-ss.GetOverheadSimpleAsTicks()
		);

		PushResultPercentileAsMicroseconds(
			resultNamesSequentialPercentile[i],
			ss.GetTagHistory(tag),
			90,
			-ss.GetOverheadSimpleAsTicks()
		);
	}

	for(uint32_t i = 0; i < numFileSizes; ++i)
	{
		auto tag = baseTag + i + 1 * 1000;
		
		PushResultMedianAsMicroseconds(
			resultNamesRandomMedian[i], 
			ss.GetTagHistory(tag),
			-ss.GetOverheadSimpleAsTicks()
		);

		PushResultPercentileAsMicroseconds(
			resultNamesRandomPercentile[i],
			ss.GetTagHistory(tag),
			90,
			-ss.GetOverheadSimpleAsTicks()
		);
	}

	std::free(synchInputBuffer);
}

