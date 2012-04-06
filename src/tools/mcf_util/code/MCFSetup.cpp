///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : MCFSetup.cpp
//   Description :
//      [TODO: Write the purpose of MCFSetup.cpp.]
//
//   Created On: 4/5/2011 4:28:07 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

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
	#ifdef DEBUG
		if (!g_hMcfCore.load("mcfcore-d.dll"))
	#else
		if (!g_hMcfCore.load("mcfcore.dll"))
	#endif
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
