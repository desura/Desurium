/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>

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
#include "mcf/MCF.h"
#include "mcf/MCFHeader.h"

#include <branding/mcfcore_version.h>
#include "mcfcore/MCFMain.h"

#include "MCFDPReporter.h"
#include "mcfcore/MCFDPReporterI.h"


gcString g_szMCFVersion("{0}.{1}.{2}.{3}", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);


namespace MCFCore
{
	CEXPORT void* FactoryBuilder(const char* name)
	{
		if (strcmp(name, MCF_FACTORY)==0)
		{
			return static_cast<void*>(new MCFCore::MCF());
		}
		if (strcmp(name, MCF_HEADER_FACTORY)==0)
		{
			return static_cast<void*>(new MCFCore::MCFHeader());
		}
		if (strcmp(name, MCF_DPREPORTER) == 0)
		{
			return static_cast<void*>(MCFCore::GetDPReporter());
		}
		return NULL;
	}

	CEXPORT void FactoryDelete(void *p, const char* name)
	{
		if (!p)
			return;

		if (strcmp(name, MCF_FACTORY)==0)
		{
			MCFCore::MCF *temp = static_cast<MCFCore::MCF*>(p);
			delete temp;
		}
		if (strcmp(name, MCF_HEADER_FACTORY)==0)
		{
			MCFCore::MCFHeader *temp = static_cast<MCFCore::MCFHeader*>(p);
			delete temp;
		}
	}
}

extern "C"
{

CEXPORT void* FactoryBuilder(const char* name)
{
	return MCFCore::FactoryBuilder(name);
}

CEXPORT void FactoryDelete(void *p, const char* name)
{
	MCFCore::FactoryDelete(p, name);
}

CEXPORT const char* GetMCFCoreVersion()
{
	return g_szMCFVersion.c_str();
}

}





