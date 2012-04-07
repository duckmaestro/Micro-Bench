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


#include "FileSystemLocalRead.h"
#include "Windows.h"
#include <cassert>
#include <sstream>
#include <iostream>

FileSystemLocalRead::FileSystemLocalRead(StopwatchStack& ss)
	: FileSystemExperiment(L"File System Local Reads", ss)
{
	_defaultNumIterations = 2;
}

FileSystemLocalRead::~FileSystemLocalRead(void)
{
}

void FileSystemLocalRead::Run(void)
{
	srand((unsigned int)time(NULL));
	uint32_t sectorSize = -1;
	uint32_t blockSize;
	uint32_t sectorsPerBlock = 1;
	StopwatchStack& ss = _stopwatch;
	void* tagSeq = L"FileSystem -- Unbuffered Read Latency Synchronous";
	void* tagRand = L"FileSystem == Unbuffered Read Latency Asynchronous";
	// make sure test files are both synchronous (for sequential reads) and asynchronous (for random ones)
	// create files without buffering (access disk directly)
	
	CreateTestFiles();
	
	sectorSize = GetLogicalSectorSize();
	assert(sectorSize > 1);
	
	blockSize = sectorSize * sectorsPerBlock;
	uint64_t log2Sector = FileSystemExperiment::GetLog2(sectorSize);
	
	for (int i = 0; i < 20/*numberFileSizes*/; i++)
	{
		// let the user at the console know we're still working.
		for(int k = 0; k < i; k++)
		{
			std::wcout << L".";
		}

		ss.ResetTagHistory(tagSeq);  //start afresh with each new file size
		ss.ResetTagHistory(tagRand);
	    int shiftAmount = (int)(i + log2Sector);
		uint64_t currentFileSize = FileSystemExperiment::GuaranteedShift(1, shiftAmount);
		uint64_t estimatedSlotSize = 1 << 14;  //size of a physical sector 
		uint64_t estimatedCacheSize = 1 << 24;  //8MB buffer cache size
		
		// output message naming:
		std::wstring seqResultName;
		{
			std::wstringstream temp;
			temp << L"Unbuffered File Read Latency Sequential, File Size:\t" << FileSystemExperiment::GuaranteedShift(1, shiftAmount) << " bytes";
			seqResultName = temp.str();
		}
		std::wstring fileName = FileSystemExperiment::GetTestFilenameBySize(currentFileSize);
		HANDLE fileHandle;
		// open file with NoBuffer
		fileHandle = (HANDLE)FileSystemExperiment::OpenFileNoBuffering(fileName);   //see "get current path link above"
		// now we have an open file directly to disk, measure time to read to it
		for(int j = 0; j < 10/*_defaultNumIterations*/; j++)
		{		
			void *seqInputBuffer;
			seqInputBuffer = malloc((size_t)estimatedSlotSize);
			if(seqInputBuffer==NULL)
			{
				throw std::exception("Malloc Returned Null, not enough memory"); 
			}
			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365541(v=vs.85).aspx

			LONG offset = (j * (int)estimatedCacheSize) % (int64_t)currentFileSize;
			if(offset==0)
			{  
				// read sequentially until we reach the end of the file, then reset at beginning
				LONG repoForHighSpot=0;
				if(SetFilePointer(fileHandle, offset, &repoForHighSpot, 0) == INVALID_SET_FILE_POINTER){
					HRESULT errorCode;
					errorCode=GetLastError();
					throw std::exception("Set Pointer Failed");			
				}
			}
			ss.Push(tagSeq);

			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
			DWORD amountRead;
			if(!ReadFile(fileHandle, seqInputBuffer, (DWORD)estimatedSlotSize, &amountRead, NULL))
			{
				HRESULT errorCode;
				errorCode = GetLastError();
				throw std::exception("File Read Failed");			
			}
			ss.Pop();	

			free(seqInputBuffer);
		}
	
		FileSystemExperiment::CloseFile(fileHandle);
		PushResultMedianAsMicroseconds(
			seqResultName, 
			(ss.GetTagHistory(tagSeq)),
			-ss.GetOverheadSimpleAsTicks()
		);

		/**************************************************************************************
		*
		* Repeat with RandIO
		*
		***************************************************************************************/

		{
			std::vector<uint64_t> quickVector;
			uint64_t bigAmountOfBytes = FileSystemExperiment::GuaranteedShift(1, 21+log2Sector);
			quickVector.push_back(bigAmountOfBytes);  //2GB should be enough to clear the fileCache 8GB would make sure
			CreateTestFiles(quickVector, false);  //don't create it if we don't need to
			std::wstring bigFileName = GetTestFilenameBySize(bigAmountOfBytes);
			FileSystemExperiment::ReadEntireFile(bigFileName);
		}


		std::wstring randResultName;
		{
			std::wstringstream temp;
			temp << L"Unbuffered File Read Latency Random, File Size:\t" << FileSystemExperiment::GuaranteedShift(1, shiftAmount) << " bytes";
			randResultName = temp.str();
		}
		
		// open file with NoBuffer
		fileHandle = (HANDLE)FileSystemExperiment::OpenFileNoBuffering(fileName);   //see "get current path link above"
		
		// now we have an open file directly to disk, measure time to read to it
		uint32_t maxRandValue = (uint32_t) (currentFileSize / estimatedCacheSize);  //can't read more slots than there are in file
		for(int j=0; j < 10 /*_defaultNumIterations*/; j++)
		{
			void *randInputBuffer;
			randInputBuffer = malloc((size_t)estimatedSlotSize);
			if(randInputBuffer==NULL){
				throw std::exception("Malloc Returned Null, not enough memory"); 
			}
			
			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365541(v=vs.85).aspx
			
			// get random "slot" to read
			LONG offset = 0;
			if(maxRandValue > 1)
			{
				uint32_t randVal = rand() % maxRandValue;
				offset = (LONG) (randVal * estimatedCacheSize);
			} 
			LONG repoForHighSpot = 0;
			if(INVALID_SET_FILE_POINTER == SetFilePointer(fileHandle, offset, &repoForHighSpot, 0))
			{
				HRESULT errorCode;
				errorCode = GetLastError();
				throw std::exception("Set Pointer Failed");			
			}
			ss.Push(tagRand);

			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
			
			DWORD amountRead;
			if(!ReadFile(fileHandle, randInputBuffer, (DWORD) estimatedSlotSize, &amountRead, NULL))
			{
				HRESULT errorCode;
				errorCode = GetLastError();
				throw std::exception("File Read Failed");			
			}
			ss.Pop();

			free(randInputBuffer);
		}
	
		FileSystemExperiment::CloseFile(fileHandle);

		PushResultMedianAsMicroseconds(
			randResultName, 
			(ss.GetTagHistory(tagRand)),
			-ss.GetOverheadSimpleAsTicks()
		);
	}
	std::wcout << L"\n" << std::endl;
}



