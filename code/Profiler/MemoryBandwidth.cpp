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


#include "MemoryBandwidth.h"
#include "Mathx.h"
#include <iostream> 
#include <sstream>

/*
*  general guidelines:
*    function to measure memory latency: 
*    turn off optimization
*    for(powers of two (or different cache sizees)){
*       for(number of test runs (default test size inherited from Experiment)
*       use malloc for the appropriate size
*         iterate through every "cache line size" number of bytes and read what is there (measure this)
*         also measure read and write (look up whether or not it is write-through)
*         free malloc'd
*       }
*     }
*      
*/

MemoryBandwidth::MemoryBandwidth(StopwatchStack& stopwatch)
	: Experiment(L"Memory Bandwidth Overhead", stopwatch)
{

}

void MemoryBandwidth::Run()
{
	StopwatchStack& ss = _stopwatch; 
	const int numIterations = 100;
	void* tagMemCopy = L"Memory -- Memory Bandwidth -- Mem Copy";
	void* tagMemRead = L"Memory -- Memory Bandwidth -- Mem Read";
	void* tagMemWrite = L"Memory -- Memory Bandwidth -- Mem Write";


	/*memory bandwidth testin'
	* two methods of testing
	*    measure the time to copy from one array to another (two malloc'd chunks)
	*    measure the time of a memcpy() system call
	*/

	//copy sizes should be greater than the greatest cache size
	for(int i = 1; i <= 1; i++) //4 sizes work? 
	{
		int numBytes = MaxCacheSize << i;

		std::wstring wMemoryTitleMemCopy;
		std::wstring wMemoryTitleRead;
		std::wstring wMemoryTitleWrite;
		{
			std::wstringstream temp;
			
			temp << L"Memory Bandwidth memcpy Bytes: " << numBytes;
			wMemoryTitleMemCopy = temp.str();
			temp.str(std::wstring());
			
			temp << L"Memory Bandwidth Read Bytes: " << numBytes;
			wMemoryTitleRead = temp.str();
			temp.str(std::wstring());

			temp << L"Memory Bandwidth Write Bytes: " << numBytes;
			wMemoryTitleWrite = temp.str();
			temp.str(std::wstring());
		}

		for(int j = 0; j < numIterations; ++j)
		{
			void *p, *q;
			p=malloc(numBytes);
			q=malloc(numBytes);   //make sure that these addresses don't map to the same places/stuff?

			if(p==NULL || q==NULL){
			std::wcout << L"Malloc Failed!\n" << std::endl;
			return;
			}  
		 
			ss.Push(tagMemCopy);
					//initialzie memory blocks?  for now we won't, since it shouldn't matter what is in the data DAC TO DO
			memcpy(q,p,numBytes);

			//test memcpy vs. read/write differene by only copying 4 bytse at a time
			/* int *source = (int *) p;
			int *dest = (int *) q;
			for(int k=0;k<(numBytes/sizeof(int)); k++){
				memcpy((void *)(dest+(k*sizeof(int))), (void *)(source+(k*sizeof(int))), sizeof(int));
			}  */
			ss.Pop();
			free(p);
			free(q);
		}

		PushResultMedianAsMicroseconds(
			wMemoryTitleMemCopy, 
			(ss.GetTagHistory(tagMemCopy)),
			-ss.GetOverheadSimpleAsTicks()
		);

		//now do the loop with hand-done memory copying
		for(int j = 0; j < numIterations; ++j)
		{
			void *p;
			p = malloc(numBytes);

			if(p == NULL)
			{
				std::wcout << L"Malloc Failed!\n" << std::endl;
				return;
			}  
		 
			//initialzie memory
			int* explorer=(int *) p;
			for(int k = 0; k < (numBytes / (int)sizeof(int)); k++)
			{
				*explorer=k;
				explorer++;
			}

			//measure read
			int manipulatedInt = 0;
			explorer = (int*) p;
			ss.Push(tagMemRead);
			for(int k = 0; k < (numBytes / (int)sizeof(int)); k++)
			{
				manipulatedInt = *(explorer) + k;
				explorer++;
			}
			ss.Pop();
			free(p);
		}

		PushResultMedianAsMicroseconds(
			wMemoryTitleRead, 
			(ss.GetTagHistory(tagMemRead)),
			-ss.GetOverheadSimpleAsTicks()
		);

		//now do the loop with hand-done memory copying
		for(int j = 0; j < numIterations; ++j)
		{
			void* p;
			p = malloc(numBytes);

			if (p == NULL) 
			{
				throw new std::exception("Malloc failed.");
			}  
		 
			// initialzie memory
			int* explorer=(int *) p;
			for(int k = 0; k < (numBytes / (int)sizeof(int)); k++)
			{
				*explorer = k;
				explorer++;
			}

			//measure write
			explorer = (int*) p;
			ss.Push(tagMemWrite);
			for(int k = 0; k < (numBytes / (int)sizeof(int)); k++)
			{
				*explorer = k + 1;
			}
			ss.Pop();
			free(p);
		}

		PushResultMedianAsMicroseconds(
			wMemoryTitleRead, 
			(ss.GetTagHistory(tagMemWrite)),
			-ss.GetOverheadSimpleAsTicks()
		);
	} 
}
