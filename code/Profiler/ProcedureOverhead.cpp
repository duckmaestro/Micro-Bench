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


#include "ProcedureOverhead.h"
#include "Mathx.h"
#include "Windows.h"

// to prevent the compiler from reusing other methods, the test procedures depend on this private static variable.
static int g_hiddenValue;

ProcedureOverhead::ProcedureOverhead(StopwatchStack& stopwatch)
	: Experiment(L"Procedure Call Overhead", stopwatch)
{

}

void ProcedureOverhead::Run()
{
	// prevent compiler from inlining these procedures.
	auto* pfn0 = &Procedure0;
	auto* pfn1 = &Procedure1;
	auto* pfn2 = &Procedure2;
	auto* pfn3 = &Procedure3;
	auto* pfn4 = &Procedure4;
	auto* pfn5 = &Procedure5;
	auto* pfn6 = &Procedure6;
	auto* pfn7 = &Procedure7;

	StopwatchStack& ss = _stopwatch;
	auto tagP0 = "P0";
	auto tagP1 = "P1";
	auto tagP2 = "P2";
	auto tagP3 = "P3";
	auto tagP4 = "P4";
	auto tagP5 = "P5";
	auto tagP6 = "P6";
	auto tagP7 = "P7";


#define PUSHRESULT PushResultMinimumAsMicroseconds


	// 0 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP0);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP0);
		pfn0();
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1); // yield early to reduce our likelihood of being switched during timing.
		}
	}
	PUSHRESULT(
		L"0 Parameters (μs)", 
		ss.GetTagHistory(tagP0),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 1 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP1);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP1);
		pfn1(i);
		ss.Pop();
	
		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"1 Parameters (μs)", 
		ss.GetTagHistory(tagP1),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 2 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP2);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP2);
		pfn2(i, 234);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"2 Parameters (μs)", 
		ss.GetTagHistory(tagP2),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 3 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP3);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP3);
		pfn3(i, 234, i);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"3 Parameters (μs)", 
		ss.GetTagHistory(tagP3),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 4 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP4);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP4);
		pfn4(i, 234, i, 456);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"4 Parameters (μs)", 
		ss.GetTagHistory(tagP4),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 5 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP5);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP5);
		pfn5(i, 234, i, 456, i);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"5 Parameters (μs)", 
		ss.GetTagHistory(tagP5),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 6 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP6);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP6);
		pfn6(i, 234, i, 456, i, 678);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"6 Parameters (μs)", 
		ss.GetTagHistory(tagP6),
		-ss.GetOverheadSimpleAsTicks()
	);

	
	// 7 parameters

	g_hiddenValue = reinterpret_cast<int>(tagP7);
	for(int i = 0; i < _defaultNumIterations; ++i)
	{
		ss.Push(tagP7);
		pfn7(i, 234, i, 456, i, 678, i);
		ss.Pop();

		if(i % 100 == 0)
		{
			Sleep(1);
		}
	}
	PUSHRESULT(
		L"7 Parameters (μs)", 
		ss.GetTagHistory(tagP7),
		-ss.GetOverheadSimpleAsTicks()
	);

}


int ProcedureOverhead::Procedure0(void)
{
	return g_hiddenValue;
}
int ProcedureOverhead::Procedure1(int a)
{
	return g_hiddenValue + a;
}
int ProcedureOverhead::Procedure2(int a, int b)
{
	return g_hiddenValue + b;
}
int ProcedureOverhead::Procedure3(int a, int b, int c)
{
	return g_hiddenValue + c;
}
int ProcedureOverhead::Procedure4(int a, int b, int c, int d)
{
	return g_hiddenValue + d;
}
int ProcedureOverhead::Procedure5(int a, int b, int c, int d, int e)
{
	return g_hiddenValue + e;
}
int ProcedureOverhead::Procedure6(int a, int b, int c, int d, int e, int f)
{
	return g_hiddenValue + f;
}
int ProcedureOverhead::Procedure7(int a, int b, int c, int d, int e, int f, int g)
{
	return g_hiddenValue + g;
}