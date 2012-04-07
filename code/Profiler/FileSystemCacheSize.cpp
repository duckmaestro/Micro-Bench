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


#include "FileSystemCacheSize.h"
#include "Windows.h"
#include <cassert>
#include <sstream>
#include <math.h>
#include <iostream>

FileSystemCacheSize::FileSystemCacheSize(StopwatchStack& ss)
	: FileSystemExperiment(L"File System Cache Size determination", ss)
{
	_defaultNumIterations = 2;
}

FileSystemCacheSize::~FileSystemCacheSize(void)
{
}

void FileSystemCacheSize::Run(void)
{
	uint32_t sectorSize = -1;	
	uint32_t blockSize;
	uint32_t sectorsPerBlock = 1;
	StopwatchStack& ss = _stopwatch;
	void* tag = L"FileSystem -- File Cache Latency";
	
	CreateTestFiles();
	
	sectorSize = GetLogicalSectorSize();
	assert(sectorSize > 1);
	
	blockSize = sectorSize * sectorsPerBlock;
	uint64_t numberFileSizes = FileSystemExperiment::NumFileSizes;
	uint64_t log2Sector = FileSystemExperiment::GetLog2(sectorSize);
	
	//create and read entirety of a really big file to make sure that fileCache is all ours (at least for a moment)
	//theoretically this could just be done with the file each time we load it (which we do too)
	{/*
		std::vector<uint64_t> quickVector;
		uint64_t bigAmountOfBytes = FileSystemExperiment::GuaranteedShift(1, 21+log2Sector);
		quickVector.push_back(bigAmountOfBytes);  //2GB should be enough to clear the fileCache 8GB would make sure
		CreateTestFiles(quickVector, false);  //don't create it if we don't need to
		std::wstring bigFileName = FileSystemExperiment::GetTestFilenameBySize(bigAmountOfBytes);
		FileSystemExperiment::ReadEntireFile(LocalDevicePath + bigFileName);
	*/}
	
	for (uint64_t i=0; i < _fileSizes.size(); i++)
	{  
		for(int k=0; k < i; k++)
		{
			std::wcout << L".";
		}

		ss.ResetTagHistory(tag);  //start afresh with each new file size    
	    int shiftAmount = (int) (i + log2Sector);
		uint64_t currentFileSize = FileSystemExperiment::GuaranteedShift(1, shiftAmount);
		
		std::wstring resultName;
		{
			std::wstringstream temp;
			temp << L"File Read Latency Normal, File Size:\t" << FileSystemExperiment::GuaranteedShift(1, shiftAmount) << " bytes";
			resultName = temp.str();
		}
		std::wstring fileName = GetTestFilenameBySize(currentFileSize);

		//read entire file in to try to reduce effect of other programs as much as possible
		FileSystemExperiment::ReadEntireFile(fileName);  
		
		HANDLE fileHandle;
		//open file normally
		fileHandle = (HANDLE)FileSystemExperiment::OpenFileNormal(fileName);  
		//now we have an open file, measure time to read to it
		for(int j = 0; j < 100/*_defaultNumIterations*/; j++)
		{
			uint64_t estimatedSlotSize = 1 << 14;  //256kb originally
			uint64_t estimatedCacheSize = 1 << 24;  //8MB * 2 to make sure we go over

			void *inputBuffer;
			inputBuffer = malloc((size_t) estimatedSlotSize);
			if(inputBuffer==NULL)
			{
				throw std::exception("Malloc Returned Null, not enough memory"); 
			}

			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365541(v=vs.85).aspx
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa364218%28v=vs.85%29.aspx
			//move file pointer by 256kb each time since the above link is the best guess I have as to cacheSlot size
			
			LONG offset = (LONG) ((j * estimatedCacheSize) % currentFileSize);
			LONG repoForHighSpot=0;
			if(offset > 0)
			{
				if(SetFilePointer(fileHandle, offset, &repoForHighSpot, 0)==INVALID_SET_FILE_POINTER)
				{
					HRESULT errorCode;
					errorCode=GetLastError();
					throw std::exception("Set Pointer Failed");			
				}
			}
			ss.Push(tag);
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
			//TODO: How deal with filePosition after whole file is read? does it start from beginning?
			DWORD amountRead;
			if(!ReadFile(fileHandle, inputBuffer, (DWORD)estimatedSlotSize, &amountRead, NULL))
			{
				HRESULT errorCode;
				errorCode = GetLastError();
				throw std::exception("File Read Failed");			
			}
			ss.Pop();

			//TODO: Use Asynch to do random IO (using file offset) -this might be doable synchronously.
			//Either way we need an OVERLAPPED Struct.
			//TODO: Initialize an OVERLAPPED structure
			//ss.Push(tagAsynch);
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
			//ReadFile(fileHandle, synchInputBuffer, sectorSize, NULL, NULL);
			//ss.Pop();

			free(inputBuffer);
			
		}
		FileSystemExperiment::CloseFile(fileHandle);
	

		PushResultMedianAsMicroseconds(
			resultName + L" Med", 
			(ss.GetTagHistory(tag)),
			-ss.GetOverheadSimpleAsTicks()
		);

		PushResultMeanAsMicroseconds(
			resultName + L" Mean", 
			(ss.GetTagHistory(tag)),
			-ss.GetOverheadSimpleAsTicks()
		);
	}
}



