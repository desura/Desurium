/*
Desura is the leading indie game distribution platform
Copyright (C) Mark Chandler <mark@moddb.com>

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

#include "Common.h"
#include "UtilFunction.h"

std::vector<UtilFunction*> *gs_FunctionList = NULL; 

void AddFunction(UtilFunction* function)
{
	if (!gs_FunctionList)
		gs_FunctionList = new std::vector<UtilFunction*>();

	gs_FunctionList->push_back(function);
}

void GetFunctionList(std::vector<UtilFunction*> &list)
{
	list = *gs_FunctionList;
}


UtilFunction::UtilFunction()
{
	s_uiLastProg = -1;
}

void UtilFunction::printProgress(MCFCore::Misc::ProgressInfo& prog)
{
	if (s_uiLastProg == prog.percent)
		return;

	s_uiLastProg = prog.percent;

	printf("\r[");

	size_t tot = (prog.percent/2);

	for (size_t x=0; x<50; x++)
	{
		if (x<tot)
			printf("=");
		else
			printf(" ");
	}

	printf("] %u%%", prog.percent);
}


void UtilFunction::mcfError(gcException& e)
{
	m_LastException = e;

	gcString err("{0}", e);
	printf("\nMCF Error: %s\n", err.c_str());
}

void UtilFunction::checkException()
{
	if (m_LastException.getErrId() != 1)
		throw m_LastException;
}
