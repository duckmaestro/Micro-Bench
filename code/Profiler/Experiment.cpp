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


#include "Experiment.h"
#include <iostream>
#include <sstream>
#include "Mathx.h"

Experiment::Experiment(std::wstring name, StopwatchStack& stopwatch)
	: _stopwatch(stopwatch)
	, _defaultNumIterations(5000)
{
	_name = name;
}

void Experiment::PushResult(std::wstring resultName, double value)
{
	_results.push_back(Result(resultName, value));
}

std::wstring Experiment::ToString(void)
{
	std::wostringstream stringBuilder;

	stringBuilder << L"::" <<  _name << L"::" << std::endl;

	for(auto iter = _results.begin(); iter != _results.end(); ++iter)
	{
		stringBuilder 
			<< "  |-" 
			<< (iter->Name) 
			<< L":\t" 
			<< (iter->Value) 
			<< std::endl;
	}

	stringBuilder << L"::" << std::endl;

	return stringBuilder.str();
}