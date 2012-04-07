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

#include "Experiment.h"
#include <vector>
#include <string>

class FileSystemExperiment : public Experiment
{
public:
	enum ReadMode
	{
		SEQUENTIAL,
		RANDOM,
	};

public:
	FileSystemExperiment(std::wstring name, StopwatchStack& ss);
	virtual ~FileSystemExperiment(void);

protected:
	/// <summary>
	/// Opens a file with software buffering disabled. 
	/// Returns handle to file.
	/// </summary>
	static void* OpenFileNoBuffering(const std::wstring& path);
	static void* OpenFileNormal(const std::wstring& path);
	static void CloseFile(void* handle);
	/// <summary>
	/// Creates a set of test files based on the sizes provided.
	/// </summary>
	void CreateTestFiles(const std::vector<uint64_t>& sizes, bool replaceExisting);
	/// <summary>
	/// Creates the default set of test files.
	/// </summary>
	void CreateTestFiles();
	std::wstring GetTestFilenameBySize(uint64_t size);
	static std::wstring GetTestFilenameBySize(uint64_t size, const std::wstring& directory);
	static uint64_t GetLog2(uint64_t num);
	static uint64_t GuaranteedShift(uint64_t num, uint64_t shiftAmt);
	static uint32_t GetLogicalSectorSize(const std::wstring& devicePath);
	uint32_t GetLogicalSectorSize();
	static uint32_t GetPhysicalSectorSize(const std::wstring& devicePath);

	static uint64_t GetFileSize(const std::wstring& path);
	static uint64_t GetFileSize(const void*const handle);
	static void ReadEntireFile(const std::wstring& path, uint32_t startPosition = 0);
	static void ReadEntireFile(
		const void*const handle, 
		const uint32_t bufferSize,
		StopwatchStack& stopwatch, 
		const void*const tag, 
		uint32_t startPosition = 0,
		ReadMode readMode = SEQUENTIAL);

protected:
	static const std::wstring TestFilenamePrefix;
	static const uint64_t NumFileSizes = 23;  //added here for generality  22*9 = 31 = 2GB (should be enough!)
	static const uint64_t DefaultBufferSize = 128 * 1024;
	
protected:
	std::wstring _devicePath;
	static std::vector<uint64_t> _fileSizes;
};
