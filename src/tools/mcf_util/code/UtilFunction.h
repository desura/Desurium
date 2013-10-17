///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : UtilFunction.h
//   Description :
//      [TODO: Write the purpose of UtilFunction.h.]
//
//   Created On: 4/5/2011 4:05:02 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_UTILFUNCTION_H
#define DESURA_UTILFUNCTION_H
#ifdef _WIN32
#pragma once
#endif

#include "mcfcore/MCFI.h"
#include "mcfcore/MCFHeaderI.h"

class UtilFunction;

void AddFunction(UtilFunction* function);
void GetFunctionList(std::vector<UtilFunction*> &list);

MCFCore::MCFI* mcfFactory();
void mcfDelFactory(void* p);

class AutoReg
{
public:
	AutoReg(UtilFunction* function)
	{
		AddFunction(function);
	}
};

#define REG_FUNCTION(class) AutoReg g_##class##_auto( new class() );

class UtilFunction
{
public:
	UtilFunction();

	virtual uint32 getNumArgs()=0;
	virtual const char* getArgDesc(size_t index)=0;

	virtual const char* getFullArg()=0;
	virtual const char getShortArg()=0;
	virtual const char* getDescription()=0;

	virtual int performAction(std::vector<std::string> &args)=0;


	void printProgress(MCFCore::Misc::ProgressInfo& prog);
	void mcfError(gcException& e);

	void checkException();

private:
	gcException m_LastException;
	uint32 s_uiLastProg;
};

#endif //DESURA_UTILFUNCTION_H
