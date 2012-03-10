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
#include "WebCore.h"
#include "../../../branding/branding.h"
#include "../../../branding/webcore_version.h"

#ifdef DESURA_OFFICAL_BUILD
	extern void* FactoryBuilderNonGpl(const char* name);
#else
	void* FactoryBuilderNonGpl(const char* name)
	{
		return NULL;
	}
#endif

gcString g_szVersion("{0}.{1}.{2}.{3}", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);

namespace WebCore
{

const char* GetWebCoreVersion()
{
	return g_szVersion.c_str();
}

gcString genUserAgent()
{
	gcString szUserAgent;

	std::string osVer = UTIL::OS::getOSString();



#ifdef DEBUG
	std::string appidStr = UTIL::OS::getConfigValue(APPID);
	std::string buildStr = UTIL::OS::getConfigValue(APPBUILD);

#ifdef WIN32
	uint32 build = 500;
#elif NIX64
	uint32 build = 520;
#else
	uint32 build = 510;
#endif

	szUserAgent = gcString("Desura/{1}.Debug ({0})", osVer, build);
#else

	std::string appidStr = UTIL::OS::getConfigValue(APPID);
	std::string buildStr = UTIL::OS::getConfigValue(APPBUILD);

	if (appidStr.size() > 0 && buildStr.size() > 0 && osVer[0])
	{
		szUserAgent = gcString("Desura/{0}.{1} ({2})", appidStr, buildStr, osVer);
	}	
	else
	{
		szUserAgent = gcString("Desura/UNKNOWN.VERSION ({0})", osVer);
		Warning("Failed to generate useragent string.\n");
	}

#endif

	return szUserAgent;
}



CEXPORT void* FactoryBuilder(const char* name)
{
	if (strcmp(WEBCORE, name) == 0)
	{
		return new WebCoreClass();
	}
	else if (strcmp(WEBCORE_VER, name) == 0)
	{
		return (void*)&GetWebCoreVersion;
	}
	else if (strcmp(WEBCORE_PASSREMINDER, name) == 0)
	{
		return (void*)&WebCoreClass::sendPassReminder;
	}
	else if (strcmp(WEBCORE_USERAGENT, name) == 0)
	{
		return (void*)&genUserAgent;
	}

	return FactoryBuilderNonGpl(name);
}

extern "C"
{
	CEXPORT void* FactoryBuilderWC(const char* name)
	{
		return FactoryBuilder(name);
	}
}

}
