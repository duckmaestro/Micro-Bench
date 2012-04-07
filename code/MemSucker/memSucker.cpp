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

// memSucker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



int _tmain(int argc, _TCHAR* argv[])
{


	// reduce process priority
		HANDLE hProcess = GetCurrentProcess();
		SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS);

		// lock thread to core 0
		HANDLE hThread = GetCurrentThread();
		//SetThreadAffinityMask(hThread, 0);
			
		// reduce main thread priority
		SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
	const unsigned int Gibby = 1 << 30;  //one gigabyte
	const unsigned int pageSize = 1 << 12; //4 kb
	void *p;
	//allocate a huge amount of memory (1 GB) and go through it performing operations to keep it in main memory
	p=malloc(Gibby);
	char *explorer = (char *) p;
	char *base=(char *) p;
	int pageNum=0;
	while(true){
		explorer=base+((pageNum*pageSize)%Gibby);
		*explorer=pageNum%256;  //mod a prime number just 'cause
		if(pageNum%Gibby==0 && pageNum > 0){
			//printf("Page Number:\t%d\n",pageNum);
			return 0;
			if(!SwitchToThread())
				{
					//printf("Yield did not work!\n");  //yield thread
			}else{
				//printf("WORKED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			}
		} 
		pageNum++;
	}
	return 0;
}

