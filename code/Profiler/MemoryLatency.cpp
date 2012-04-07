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


#include "MemoryLatency.h"
#include "Mathx.h"
#include <iostream>
#include <sstream>

// http://stackoverflow.com/questions/4707384/disable-all-types-of-optimizations-in-vs2010
// http://msdn.microsoft.com/en-us/library/chh3fb0k(VS.80).aspx
	
MemoryLatency::MemoryLatency(StopwatchStack& stopwatch)
	: Experiment(L"Memory Latency Overhead", stopwatch)
{

}

void MemoryLatency::Run()
{
	StopwatchStack& ss = _stopwatch; 
	const int numIterations = 200;
	void* tag = L"Memory -- Memory Latency";

	const int numMemoryReads = 100000;  //100,000 reads?

/*
*  general guidelines:
*    function to measure memory latency: 
*    turn off optimization
*    for(powers of two (or different cache sizees)){
*       for(number of test runs (default test size inherited from Experiment){
*         use malloc for the appropriate size
*         initialize malloc'd chunk to contain pointers which all point to "stride" further down (cache size in this case)
*         go from pointer to pointer for a specified number of times in the memory and record
*         free malloc'd
*       }
*     }
*      
*/
	//starting at cache-size bytes, double size for each test round until we have doubel the largest cache (not main memory)

	for(int i = CacheLineSize; i < (MaxCacheSize  << 2); i = i << 1)
	{ 
		std::wstring wMemoryTitle;
		{
			std::wstringstream temp;
			temp << L"Memory Latency Bytes: " << i;
			wMemoryTitle = temp.str();
		}

		ss.ResetTagHistory(tag);
		for(int j = 0; j < numIterations; ++j)
		{
			void* p;
			p = malloc(i);
	   
			if(p==NULL)
			{
				std::wcout << L"Malloc Failed!\n" << std::endl;
				return;
			}  

			// iterate through malloc once so that cache is flushed
			// ARRAY VERSION
			//int blah = 128;
			//int* explorerArray = new int[i/4];
			//int* base = explorerArray;
			//// initialize all of memory to contain pointers pointing to themselves
			//for(int k=0;k <(i/4) ;k++){
			//	explorerArray[k]=(int)&explorerArray[(k+_cacheLineSize)%(i/4)];
			//}
			//int* explorer=explorerArray;
			//ss.Push(L"memoryTitle");
			//for(int k=0;k<1000;k++){
			//	explorer=(int *)*explorer;
			//}
			//

			// MALLOC VERSION
			int** explorer = (int**) p;
			int** base = explorer;

			// initialize all of memory to contain pointers pointing to themselves
			for(int k = 0; k < (i / 4); k++)
			{
				*explorer = (int*) (base + ((k + CacheLineSize) % (i / 4)));
				explorer++;
			}
			explorer=base;
			ss.Push(tag);
			for(int k = 0; k < numMemoryReads; k += 20)
			{
				// doing this "unrolled" 20 at a time to reduce loop overhead
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
				explorer = (int**) *explorer;
			}
			ss.Pop();
			//delete [] explorerArray;
			base = NULL;
			explorer = NULL;
			free(p);
		}

		PushResultMedianAsMicroseconds(
			wMemoryTitle, 
			(ss.GetTagHistory(tag)),
			-ss.GetOverheadSimpleAsTicks()
		);

	}
}

#pragma optimize( "", on )
