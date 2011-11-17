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
#include "ToolManager.h"

#include "User.h"

#include "ToolInfo.h"
#include "ToolTransaction.h"
#include "ToolInstallThread.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

bool CheckSunJava()
{
	std::string res = UTIL::LIN::getCmdStdout("which java");
	
	if (res.size() == 0)
		return false;
		
	res = UTIL::LIN::getCmdStdout("java -version 2>&1 | grep \"HotSpot\"");
	return (res.size() != 0);
}

bool CheckJava()
{
	std::string res = UTIL::LIN::getCmdStdout("which java");
	return (res.size() != 0);
}

bool CheckMono()
{
	std::string res = UTIL::LIN::getCmdStdout("which mono");
	return (res.size() != 0);	
}

bool CheckAir()
{
	return UTIL::FS::isValidFile("/opt/Adobe AIR/Versions/1.0/Adobe AIR Application Installer");
	
}

typedef bool (*CheckFn)();

typedef struct
{
	gcString name;
	CheckFn funct;
} NonInstallInfo_s;

NonInstallInfo_s g_NonInstallInfo[] = 
{
	{"PRECHECK_SUN_JAVA", CheckSunJava},
	{"PRECHECK_JAVA", CheckJava},
	{"PRECHECK_MONO", CheckMono},
	{"PRECHECK_AIR", CheckAir},
	{"", NULL},
};


namespace UserCore
{
void ToolManager::onSpecialCheck(WCSpecialInfo &info)
{
	if (info.name == "msicheck")
	{
		info.handled = true;

		std::vector<std::string> out;
		UTIL::STRING::tokenize(info.result, out, " ");
		
		info.result = "!! Not Installed !!";
		
		if (out.size() < 2)
			return;
		
		bool is64 = (out[0] == "64");

		gcString cmd("./bin/findlib.sh {0} {1}", out[1], is64?"64":"32");
		std::string res = UTIL::LIN::getCmdStdout(cmd.c_str(), 2);

		if (res.size() == 0 || !UTIL::FS::isValidFile(res.c_str()))
			return;

		UTIL::FS::Path filePath(gcString(UTIL::OS::getAppDataPath()).c_str(), "", false);
		filePath += gcString("game_lib{0}", out[0]);

		UTIL::FS::recMakeFolder(filePath);
		filePath += UTIL::FS::File(out[1]);		

		std::string full = filePath.getFullPath();

		int symres = symlink(res.c_str(), full.c_str());
		
		if (symres != 0)
			return;

		info.result = full;
	}
}

void ToolManager::symLinkTools(std::vector<DesuraId> &list, const char* path)
{
	if (!path)
		return;
	
	UTIL::FS::recMakeFolder(path);
	
	for (size_t x=0; x<list.size(); x++)
	{
		ToolInfo* info = findItem(list[x].toInt64());

		if (!info || !info->isInstalled())
			continue;

		UTIL::FS::Path fp(info->getExe(), "", true);

		if (!UTIL::FS::isValidFile(fp))
			continue;

		UTIL::FS::Path dp(path, "", false);
		dp += fp.getFile();
		
		std::string src = fp.getFullPath();
		std::string dest = dp.getFullPath();

		int res = symlink(src.c_str(), dest.c_str());
		
		if (res != 0)
			Debug(gcString("Failed to sym link: [{0}] to [{1}]\n", src, dest));
	}
}

int ToolManager::hasNonInstallableTool(std::vector<DesuraId> &list)
{
	for (size_t x=0; x<list.size(); x++)
	{
		UserCore::ToolInfo* info = findItem(list[x].toInt64());

		if (!info)
			continue;
		
		size_t y=0;
		
		while (g_NonInstallInfo[y].funct)
		{
			if (g_NonInstallInfo[y].name == info->getArgs())
			{
				if (!g_NonInstallInfo[y].funct())
					return y;
			}
			
			y++;
		}
	}	
	
	return -1;
}

void ToolManager::postParseXml()
{
	BaseManager<ToolInfo>::for_each([](ToolInfo* info)
	{
		size_t y=0;
		
		while (g_NonInstallInfo[y].funct)
		{
			if (g_NonInstallInfo[y].name == info->getArgs())
			{
				info->setInstalled(true);
				break;
			}
			
			y++;
		}
	});
}

void ToolManager::addJSTool(UserCore::Item::ItemInfo* item, uint32 branchId, gcString name, gcString exe, gcString args, gcString res)
{
}

void ToolManager::findJSTools(UserCore::Item::ItemInfo* item)
{
}

bool ToolManager::initJSEngine()
{
	return true;
}

bool ToolManager::loadJSEngine()
{
	return true;
}

void ToolManager::destroyJSEngine()
{
}

void ToolManager::unloadJSEngine(bool forced)
{
}




}
