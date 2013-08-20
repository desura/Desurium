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

#include "Common.h"
#include "McfInit.h"
#include "SharedObjectLoader.h"
#include "Log.h"

typedef void* (*BFACT)(const char*);
typedef void (*DFACT)(void*, const char*);



SharedObjectLoader g_pMCFCore;

BFACT buildFactory = NULL;
DFACT delFactory = NULL;


void initFactory()
{
	bool res = false;

#if defined(WIN32) && !defined(DEBUG) && defined(DESURA_OFFICAL_BUILD)
	char message[255] = {0};
	if (UTIL::WIN::validateCert(L".\\mcfcore.dll", message, 255) != ERROR_SUCCESS)
	{
		Warning(gcString("Cert validation failed on mcfcore.dll: {0}\n", message));
		exit(-1);
	}
#endif

#ifdef WIN32
	res = g_pMCFCore.load("mcfcore.dll");
#else
	res = g_pMCFCore.load("libmcfcore.so");
#endif

    if (!res)
        exit(-1);

	buildFactory = g_pMCFCore.getFunction<BFACT>("FactoryBuilder");

    if (!buildFactory)
        exit(-1);

	delFactory = g_pMCFCore.getFunction<DFACT>("FactoryDelete");

    if (!delFactory)
        exit(-1);

	RegDLLCB_MCF regMsg = g_pMCFCore.getFunction<RegDLLCB_MCF>("RegDLLCB_MCF");

	if (regMsg)
		InitLogging(regMsg);
}


MCFCore::MCFI* mcfFactory()
{
    if (!buildFactory)
		initFactory();

	if (!buildFactory)
        return NULL;

    void* temp = buildFactory(MCF_FACTORY);

    return static_cast<MCFCore::MCFI*>(temp);
}

void mcfDelFactory(void* p)
{
    if (!delFactory)
        return;

	delFactory(p, MCF_FACTORY);
}


void shutDownFactory()
{
	buildFactory = NULL;
	delFactory = NULL;
	DestroyLogging();
}

namespace MCFCore
{
	CEXPORT void* FactoryBuilder(const char* name)
	{
		if (!buildFactory)
			initFactory();

		if (!buildFactory)
			return NULL;

		return buildFactory(name);
	}

	CEXPORT void FactoryDelete(void *p, const char* name)
	{
		if (!delFactory)
			return;

		delFactory(p, name);
	}
}