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
#include "sqlite3x.hpp"
#include "sql/ToolManagerSql.h"

#include "DownloadToolTask.h"
#include "ToolTransaction.h"
#include "ToolInstallThread.h"

#include "ScriptCoreI.h"
#include "gcJSBase.h"

namespace UserCore
{
	
void ToolManager::onSpecialCheck(WCSpecialInfo &info)
{
	if (info.name == "msicheck")
	{
		info.handled = true;

		if (UTIL::WIN::isMsiInstalled(info.result.c_str()))
		{
			UTIL::FS::Path filePath(m_pUser->getAppDataPath(), "", false);

			filePath += "tools";
			filePath += "installcheck";

			UTIL::FS::recMakeFolder(filePath);
			filePath += UTIL::FS::File(info.result);

			try
			{
				UTIL::FS::FileHandle fh(filePath, UTIL::FS::FILE_WRITE);
				fh.write("installed", 9);
				fh.close();

				info.result = filePath.getFullPath();
			}
			catch (gcException)
			{
				Warning(gcString("Failed to open file {0} for Tool Msi Check.", filePath.getFullPath()));
			}
		}
		else
		{
			info.result = "!! Not Installed !!";
		}
	}
	else if (info.name == "javacheck")
	{
		info.handled = true;

		std::string cur = UTIL::WIN::getRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Runtime Environment\\CurrentVersion");
		std::string home = UTIL::WIN::getRegValue(gcString("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Runtime Environment\\{0}\\JavaHome", cur));

		char szJ[2] = {0};
		szJ[0] = home.back();

		uint32 cVer = atoi(info.result.c_str());
		uint32 jVer = atoi(szJ);

		if (cVer != 0 && cVer >= jVer)
			info.result = home;
		else
			info.result = "!! Not Installed !!";
	}
	else if (info.name == "dotnetcheck")
	{
		info.handled = true;

		std::vector<std::string> list;
		UTIL::STRING::tokenize(info.result, list, ".");

		uint32 major = 0;
		uint32 minor = 0;
		bool client = false;

		if (list.size() >= 1)
			major = atoi(list[0].c_str());
		if (list.size() >= 2)
			minor = atoi(list[1].c_str());
		if (list.size() >= 3)
			client = list[2] == "c";

		if (UTIL::WIN::isDotNetVersionInstalled(major, minor, client))
		{
			UTIL::FS::Path filePath(m_pUser->getAppDataPath(), "", false);

			filePath += "tools";
			filePath += "installcheck";

			UTIL::FS::recMakeFolder(filePath);
			filePath += UTIL::FS::File(gcString("dot_net_") + info.result);

			try
			{
				UTIL::FS::FileHandle fh(filePath, UTIL::FS::FILE_WRITE);
				fh.write("installed", 9);
				fh.close();

				info.result = filePath.getFullPath();
			}
			catch (gcException)
			{
				Warning(gcString("Failed to open file {0} for Tool .Net Check.", filePath.getFullPath()));
			}
		}
		else
		{
			info.result = "!! Not Installed !!";
		}
	}
	else if (info.name == "temp")
	{
		UTIL::FS::Path path(m_pUser->getAppDataPath(), "", false);
		path += "temp";
		
		time_t t = time(NULL) + GetTickCount();
		gcSleep(1000);
		path += gcString("{0}", t);

		UTIL::FS::recMakeFolder(path);

		info.result = path.getFullPath();
		info.handled = true;
	}
}	
	
void ToolManager::onPipeDisconnect()
{
	m_bDeleteThread = true;
}

}
