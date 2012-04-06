///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : UtilFunction.cpp
//   Description :
//      [TODO: Write the purpose of UtilFunction.cpp.]
//
//   Created On: 4/5/2011 4:14:17 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

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
