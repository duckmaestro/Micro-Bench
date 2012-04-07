
#include "FileSystemLocalReadA.h"
#include "Windows.h"
#include <cassert>
#include <sstream>
#include <iostream>

FileSystemLocalReadA::FileSystemLocalReadA(StopwatchStack& ss)
	: FileSystemExperiment(L"File System Local ReadsA", ss)
{
}

FileSystemLocalReadA::~FileSystemLocalReadA(void)
{
}

void FileSystemLocalReadA::Run(void)
{
	srand(time(NULL));
	uint32_t sectorSize=-1;
	uint32_t blockSize;
	uint32_t sectorsPerBlock=1;
	StopwatchStack& ss = _stopwatch;
	void* tagSeq = L"FileSystem -- Unbuffered Read Latency Synchronous";
	void* tagRand = L"FileSystem == Unbuffered Read Latency Asynchronous";
	//make sure test files are both synchronous (for sequential reads) and asynchronous (for random ones)
	//create files without buffering (access disk directly)
	CreateTestFiles();
	sectorSize = FileSystemExperiment::GetLogicalSectorSize(FileSystemExperiment::LocalDevicePath);
	assert(sectorSize > 1);
	blockSize=sectorSize*sectorsPerBlock;
	uint64_t numberFileSizes=FileSystemExperiment::GetNumberFileSizes();
	uint64_t log2Sector = FileSystemExperiment::GetLog2(sectorSize);
	//overlapped stuff
	
	//create and read entirety of a really big file to make sure that the hard drive cache is cleared
	/*{
		std::vector<uint64_t> quickVector;
		uint64_t bigAmountOfBytes = FileSystemExperiment::GuaranteedShift(1, 25);
		quickVector.push_back(bigAmountOfBytes);  //2GB should be enough to clear the fileCache 8GB would make sure
		CreateTestFiles(quickVector, false);  //don't create it if we don't need to
		std::wstring bigFileName = FileSystemExperiment::GetTestFilenameBySize(bigAmountOfBytes);
		FileSystemExperiment::ReadEntireFile(bigFileName);
	}*/

	for (unsigned int i=18; i < 21/*numberFileSizes*/; i++){
		for(int k=0; k<i; k++){
			std::wcout << L".";
		}
		ss.ResetTagHistory(tagSeq);  //start afresh with each new file size
		ss.ResetTagHistory(tagRand);
	    int shiftAmount=i+log2Sector;
		uint64_t currentFileSize = FileSystemExperiment::GuaranteedShift(1, shiftAmount);
		uint64_t estimatedSlotSize=1 << 14;  //size of a physical sector 
		uint64_t estimatedCacheSize=1 << 24;  //8MB buffer cache size
		//output message naming:
		std::wstring seqResultName;
		{
			std::wstringstream temp;
			temp << L"Unbuffered File Read Latency Sequential, File Size:\t" << FileSystemExperiment::GuaranteedShift(1, shiftAmount) << " bytes";
			seqResultName = temp.str();
		}
		std::wstring fileName = FileSystemExperiment::GetTestFilenameBySize(currentFileSize);
		HANDLE fileHandle;
		//open file with NoBuffer
		fileHandle = (HANDLE)FileSystemExperiment::OpenFileNoBufferingA(fileName);   //see "get current path link above"
		//now we have an open file directly to disk, measure time to read to it
		for(int j=0; j < 10/*_defaultNumIterations*/; j++){		
			void *seqInputBuffer;
			seqInputBuffer = malloc(estimatedSlotSize);
			if(seqInputBuffer==NULL){
				throw std::exception("Malloc Returned Null, not enough memory"); 
			}
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365541(v=vs.85).aspx

			LONG offset=(j*estimatedCacheSize)%currentFileSize;
			if(offset==0){  //read sequentially until we reach the end of the file, then reset at beginning
				LONG repoForHighSpot=0;
				/*if(SetFilePointer(fileHandle, offset, &repoForHighSpot, 0)==INVALID_SET_FILE_POINTER){
					HRESULT errorCode;
					errorCode=GetLastError();
					throw std::exception("Set Pointer Failed");			
				}*/
			}
			{
				std::vector<uint64_t> quickVector;
				uint64_t bigAmountOfBytes = FileSystemExperiment::GuaranteedShift(1, 25);
				quickVector.push_back(bigAmountOfBytes);  //2GB should be enough to clear the fileCache 8GB would make sure
				CreateTestFiles(quickVector, false);  //don't create it if we don't need to
				std::wstring bigFileName = FileSystemExperiment::GetTestFilenameBySize(bigAmountOfBytes);
				FileSystemExperiment::ReadEntireFile(bigFileName);
			}
			OVERLAPPED overlappy;
			overlappy.Offset=offset;
			overlappy.OffsetHigh=0;
			overlappy.Pointer=0;
			overlappy.hEvent=CreateEvent(NULL, FALSE, FALSE, L"RoboEvent");
			ss.Push(tagSeq);
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
			//TODO: How deal with filePosition after whole file is read? does it start from beginning?
			DWORD amountRead;
			ReadFile(fileHandle, seqInputBuffer, estimatedSlotSize, NULL, &overlappy);
			/*if(!ReadFile(fileHandle, seqInputBuffer, estimatedSlotSize, NULL, &overlappy))
			{
				HRESULT errorCode;
				errorCode=GetLastError();
				throw std::exception("File Read Failed");			
			} */
			DWORD waitResult=WaitForSingleObject(overlappy.hEvent, 10000);  //see if this works
			if(waitResult!=WAIT_OBJECT_0){
				throw std::exception("ah!");
			}
			ss.Pop();	

			free(seqInputBuffer);
		}
	
		FileSystemExperiment::CloseFile(fileHandle);
		PushResultMeanAsMicroseconds(
			seqResultName, 
			(ss.GetTagHistory(tagSeq)),
			-ss.GetOverheadSimpleAsTicks()
		);

		/**************************************************************************************
		*
		* Repeat with RandIO
		*
		***************************************************************************************/

		/*{
			std::vector<uint64_t> quickVector;
			uint64_t bigAmountOfBytes = FileSystemExperiment::GuaranteedShift(1, 21+log2Sector);
			quickVector.push_back(bigAmountOfBytes);  //2GB should be enough to clear the fileCache 8GB would make sure
			CreateTestFiles(quickVector, false);  //don't create it if we don't need to
			std::wstring bigFileName = FileSystemExperiment::GetTestFilenameBySize(bigAmountOfBytes);
			FileSystemExperiment::ReadEntireFile(bigFileName);
		}*/


		std::wstring randResultName;
		{
			std::wstringstream temp;
			temp << L"Unbuffered File Read Latency Random, File Size:\t" << FileSystemExperiment::GuaranteedShift(1, shiftAmount) << " bytes";
			randResultName = temp.str();
		}
		//open file with NoBuffer
		fileHandle = (HANDLE)FileSystemExperiment::OpenFileNoBuffering(fileName);   //see "get current path link above"
		//now we have an open file directly to disk, measure time to read to it
		uint32_t maxRandValue=currentFileSize/estimatedCacheSize;  //can't read more slots than there are in file
		for(int j=0; j < 0 /*_defaultNumIterations*/; j++){
			void *randInputBuffer;
			randInputBuffer = malloc(estimatedSlotSize);
			if(randInputBuffer==NULL){
				throw std::exception("Malloc Returned Null, not enough memory"); 
			}
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365541(v=vs.85).aspx
			//get random "slot" to read
			LONG offset=0;
			if(maxRandValue>1){
				uint32_t randVal = rand()%maxRandValue;
				offset=randVal*estimatedCacheSize;
			} 
			LONG repoForHighSpot=0;
			if(SetFilePointer(fileHandle, offset, &repoForHighSpot, 0)==INVALID_SET_FILE_POINTER){
				HRESULT errorCode;
				errorCode=GetLastError();
				throw std::exception("Set Pointer Failed");			
			}
			ss.Push(tagRand);
			//http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467(v=vs.85).aspx
			//TODO: How deal with filePosition after whole file is read? does it start from beginning?
			DWORD amountRead;
			if(!ReadFile(fileHandle, randInputBuffer, estimatedSlotSize, &amountRead, NULL))
			{
				HRESULT errorCode;
				errorCode=GetLastError();
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



