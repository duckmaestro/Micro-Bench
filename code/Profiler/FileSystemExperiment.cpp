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


#include "FileSystemExperiment.h"
#include "Windows.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>

const std::wstring FileSystemExperiment::TestFilenamePrefix = L"TestFile";
std::vector<uint64_t> FileSystemExperiment::_fileSizes;

FileSystemExperiment::FileSystemExperiment(std::wstring name, StopwatchStack& ss)
	: Experiment(name, ss)
	, _devicePath(L"C:\\")
{
	if(_fileSizes.empty())
	{
		uint32_t secSize = GetLogicalSectorSize(_devicePath);
		uint32_t logSecSize = (uint32_t) GetLog2(secSize);  // HACK: assumes secSize is power of 2 (fair assumption)
		for(uint32_t i = 0; i < NumFileSizes; ++i)
		{
			_fileSizes.push_back(GuaranteedShift(1, (i+logSecSize)));
		}
	}
}

FileSystemExperiment::~FileSystemExperiment(void)
{
}

void* FileSystemExperiment::OpenFileNoBuffering(const std::wstring& path)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/cc644950(v=vs.85).aspx
	
	HANDLE fileHandle 
		= ::CreateFile(
			path.c_str(), 
			GENERIC_READ, 
			FILE_SHARE_READ, 
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, 
			NULL
		)
	;
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		HRESULT result = HRESULT_FROM_WIN32(GetLastError());
		throw std::exception();
	}

	return fileHandle;
}

//TODO:  Note - I copied this to a new file rather than adding a paramater to the existing one so as not to mess up anything already written
 void* FileSystemExperiment::OpenFileNormal(const std::wstring& path)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/cc644950(v=vs.85).aspx
	
	HANDLE fileHandle 
		= ::CreateFile(
			path.c_str(), 
			GENERIC_READ, 
			0, 
			NULL, 
			OPEN_EXISTING,         //TODO: might want to change this?
			FILE_ATTRIBUTE_NORMAL, 
			NULL
		)
	;
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		HRESULT result = HRESULT_FROM_WIN32(GetLastError());
		throw std::exception();
	}

	return fileHandle;
}

void FileSystemExperiment::CloseFile(void* handle)
{
	assert(handle != NULL);
	::CloseHandle(handle);
}

std::wstring FileSystemExperiment::GetTestFilenameBySize(uint64_t size)
{
	return FileSystemExperiment::GetTestFilenameBySize(size, _devicePath);
}

std::wstring FileSystemExperiment::GetTestFilenameBySize(uint64_t size, const std::wstring& directory)
{
	std::wstringstream filename;
	filename << directory << TestFilenamePrefix << size;
	return filename.str();
}

// used for bit shifts of 31+ bits  (bizarre)
uint64_t FileSystemExperiment::GetLog2(uint64_t num)
{
	uint64_t log2Count = 1;
	while((num >> log2Count) > 1) 
	{
		log2Count++;
	}
	return log2Count;
}

uint64_t FileSystemExperiment::GuaranteedShift(uint64_t num, uint64_t shiftAmt)
{
	num = (1 << (shiftAmt % 31));
	num = num << ((shiftAmt / 31) * 31);
	return num;
}

void FileSystemExperiment::CreateTestFiles(const std::vector<uint64_t>& sizes, bool replaceExisting)
{
	for(auto iter = sizes.begin(); iter != sizes.end(); ++iter)
	{
		auto size = *iter;
		std::wstring filename = _devicePath + GetTestFilenameBySize(size);
		std::fstream file;
		file.open(filename, std::fstream::in);
		if(file.is_open() && !replaceExisting)
		{
			continue; // file exists
		}
		file.open(filename, std::fstream::out | std::fstream::binary);
		assert(file.is_open());
		
		uint64_t chunk  = 'a';
		uint64_t numChunksToWrite = size / sizeof(chunk);
		for(decltype(size) i = 0; i < numChunksToWrite; ++i)
		{
			file.write(reinterpret_cast<char*>(&chunk), sizeof(chunk));
		}
	}
}

void FileSystemExperiment::CreateTestFiles()
{
	CreateTestFiles(_fileSizes, false);  
}

uint32_t FileSystemExperiment::GetLogicalSectorSize()
{
	return FileSystemExperiment::GetLogicalSectorSize(_devicePath);
}

uint32_t FileSystemExperiment::GetLogicalSectorSize(const std::wstring& devicePath)
{
	DWORD sectorsPerCluster;
	DWORD bytesPerSector;
	DWORD numberOfFreeClusters;
	DWORD totalNumberOfClusters;
	
	BOOL success;
	success 
		= GetDiskFreeSpace(
			devicePath.c_str(),
			&sectorsPerCluster,
			&bytesPerSector,
			&numberOfFreeClusters,
			&totalNumberOfClusters
		)
	;

	if(!success)
	{
		HRESULT result = HRESULT_FROM_WIN32(GetLastError());
		throw std::exception();
	}

	return bytesPerSector;
}

uint32_t FileSystemExperiment::GetPhysicalSectorSize(const std::wstring& devicePath)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/cc644950(v=vs.85).aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ff800830(v=vs.85).aspx
	throw std::exception("not implemented yet.");
}

uint64_t FileSystemExperiment::GetFileSize(const std::wstring& path)
{
	HANDLE hFile = FileSystemExperiment::OpenFileNormal(path);
	uint64_t size = GetFileSize(hFile);
	CloseFile(hFile);
	return size;
}

uint64_t FileSystemExperiment::GetFileSize(const void*const handle)
{
	HANDLE hFile = (HANDLE)handle;
	LARGE_INTEGER size;
	if(!GetFileSizeEx(hFile, &size))
	{
		HRESULT result = HRESULT_FROM_WIN32(GetLastError());
		throw new std::exception();
	}
	
	return size.QuadPart;
}

void FileSystemExperiment::ReadEntireFile(const std::wstring& path, uint32_t startPosition)
{
	// open file
	HANDLE hFile = (HANDLE) FileSystemExperiment::OpenFileNoBuffering(path.c_str());
	assert(hFile != INVALID_HANDLE_VALUE);

	// read
	StopwatchStack stopwatchDummy;
	ReadEntireFile(hFile, DefaultBufferSize, stopwatchDummy, 0, startPosition);

	// close
	FileSystemExperiment::CloseFile(hFile);
}

void FileSystemExperiment::ReadEntireFile(
		const void*const fileHandle, 
		const uint32_t bufferSize,
		StopwatchStack& stopwatch, 
		const void*const tag, 
		uint32_t startPosition,
		ReadMode readMode)
{
	// check handle
	HANDLE hFile = (HANDLE)fileHandle;
	assert(hFile != INVALID_HANDLE_VALUE);
	
	// allocate buffer
	void* buffer = std::malloc(bufferSize);

	// seek
	{
		DWORD currPosition = SetFilePointer(hFile, startPosition, NULL, FILE_BEGIN);
		assert(currPosition != INVALID_SET_FILE_POINTER);
	}

	// grab file size
	uint64_t fileSize = GetFileSize(hFile);
	assert(fileSize > 0);

	// read
	uint64_t totalRead = 0;
	while(totalRead < fileSize)
	{
		DWORD amountRead = 0;
		
		stopwatch.Push(tag);
		::ReadFile(hFile, buffer, bufferSize, &amountRead, NULL);
		stopwatch.Pop();
		
		totalRead += amountRead;
		if(amountRead == 0 && readMode == SEQUENTIAL)
		{
			DWORD currPosition = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
			assert(currPosition != INVALID_SET_FILE_POINTER);
		}
		else if(readMode == RANDOM)
		{
			uint64_t randomPosition = ((uint64_t)rand() << 9) % fileSize;
			DWORD currPosition = SetFilePointer(hFile, (LONG)(randomPosition % UINT32_MAX), NULL, FILE_BEGIN);
			assert(currPosition != INVALID_SET_FILE_POINTER);
		}
	}

	// cleanup
	std::free(buffer);
}