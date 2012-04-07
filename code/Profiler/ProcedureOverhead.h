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

class ProcedureOverhead : public Experiment
{
public:
	ProcedureOverhead(StopwatchStack& stopwatch);
	virtual void Run(void);

private:
	static int Procedure0(void);
	static int Procedure1(int a);
	static int Procedure2(int a, int b);
	static int Procedure3(int a, int b, int c);
	static int Procedure4(int a, int b, int c, int d);
	static int Procedure5(int a, int b, int c, int d, int e);
	static int Procedure6(int a, int b, int c, int d, int e, int f);
	static int Procedure7(int a, int b, int c, int d, int e, int f, int g);

};

