/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef DESURA_PROGESS_INFO_H
#define DESURA_PROGESS_INFO_H
#ifdef _WIN32
#pragma once
#endif

typedef struct
{
	uint32 rate;
	uint8 flag;
	uint8 hour;
	uint8 min;
	uint8 percent;
} MCFProg_s;

typedef union 
{
	uint64 value;
	MCFProg_s prog;
} Prog_u;

namespace MCFCore
{
//! Namespace for all utillity classes used by MCFCore. Needed to be named misc instead of utill due to namespace class with util libraries
namespace Misc
{

//! This class holds progress information for MCF processes
//! and is used in the progress events
//!
class ProgressInfo
{
public:
	enum
	{
		FLAG_NONE,			//!< No flags
		FLAG_INITFINISHED,	//!< Setup stage is finished
		FLAG_FINALIZING,	//!< Onto final stage
	};

	ProgressInfo()
	{
		min = 0;
		hour = 0;
		rate = 0;
		percent = 0;
		flag = 0;
		doneAmmount = 0;
		totalAmmount = 0;
	}

	ProgressInfo(ProgressInfo* i)
	{
		min = 0;
		hour = 0;
		rate = 0;
		percent = 0;
		flag = 0;
		doneAmmount = 0;
		totalAmmount = 0;

		if (i)
		{
			min = i->min;
			hour= i->hour;
			rate= i->rate;
			percent = i->percent;
			flag = i->flag;
			doneAmmount = i->doneAmmount;
			totalAmmount = i->totalAmmount;
		}
	}

	//for ipc communication
	ProgressInfo(uint64 prog)
	{
		Prog_u u;
		u.value = prog;

		min = u.prog.min;
		hour = u.prog.hour;
		rate = u.prog.rate;
		percent = u.prog.percent;
		flag = u.prog.flag;
		doneAmmount = 0;
		totalAmmount = 0;
	}

	uint64 toInt64()
	{
		Prog_u u;

		u.prog.min = min;
		u.prog.hour = hour;
		u.prog.rate = rate;
		u.prog.percent = percent;
		u.prog.flag = flag;

		return u.value;
	}

	uint64 doneAmmount;
	uint64 totalAmmount;
	uint32 rate;
	uint8 flag;
	uint8 hour;
	uint8 min;
	uint8 percent;
};


}
}

#endif
