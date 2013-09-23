/*
Desura is the leading indie game distribution platform
Copyright (C) 2013 Mark Chandler (Desura Net Pty Ltd)

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
#include "SharedObjectLoader.h"

#include "mcfcore/MCFI.h"
#include "mcfcore/MCFHeaderI.h"

typedef void* (*BFACT)(const char*);
typedef void (*DFACT)(void*, const char*);
typedef const char* (*VFACT)();


BFACT buildFactory = NULL;
DFACT delFactory = NULL;

SharedObjectLoader g_hMcfCore;

const char* GetMcfVersion()
{
	VFACT mcfVersion = g_hMcfCore.getFunction<VFACT>("GetMCFCoreVersion");

	if (!mcfVersion)
		return "Null";

	return mcfVersion();
}

void InitFactory()
{
#ifdef NIX
    if (!g_hMcfCore.load("libmcfcore.so"))
    {
        fprintf(stderr, "Cannont open library: %s\n", dlerror());
        printf("Error loading mcfcore.so\n");
        exit(-1);
    }
#else
		if (!g_hMcfCore.load("mcfcore.dll"))
		{
			printf("Error loading mcfcore.dll\n");
			exit(-1);
		}
#endif

    buildFactory = g_hMcfCore.getFunction<BFACT>("FactoryBuilder");
    if (!buildFactory)
    {
        printf("Error loading buildfactory\n");
        exit(-1);
    }

	delFactory = g_hMcfCore.getFunction<DFACT>("FactoryDelete");
    if (!delFactory)
    {
        printf("Error loading del factory\n");
        exit(-1);
    }
}

MCFCore::MCFI* mcfFactory()
{
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
