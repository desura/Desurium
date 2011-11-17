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
#include "ToolInstallThread.h"
#include "ToolTransaction.h"
#include "ToolInfo.h"

gcException IPCToolMain::installTool(UserCore::ToolInfo* info)
{
	gcString exe(info->getExe());
	gcString args(info->getArgs());
	
	if (!UTIL::FS::isValidFile(exe.c_str()))
		return gcException(ERR_INVALIDFILE);
	
	if (exe.size() > 0 && args == "GAME_LIBRARY")
	{
		size_t pos = exe.find_last_of(".bz2");
		
		if (pos == (exe.size()-1))
		{
			UTIL::FS::Path pd(exe.c_str(), "", true);
			pd += UTIL::FS::File(info->getName());
			
			gcString dest = pd.getFullPath();
			uint64 size = UTIL::FS::getFileSize(exe.c_str());
			
			try
			{
				UTIL::FS::FileHandle srcFh(exe.c_str(), UTIL::FS::FILE_READ);
				UTIL::FS::FileHandle destFh(dest.c_str(), UTIL::FS::FILE_WRITE);
				
				UTIL::MISC::BZ2Worker bz2(UTIL::MISC::BZ2_DECOMPRESS);
			
				srcFh.read(size, [&bz2, &destFh](const unsigned char* buff, uint32 size) -> bool
				{
					UTIL::FS::FileHandle *pDest = &destFh;
					
					bz2.write((const char*)buff, size, [&pDest](const unsigned char* buff, uint32 size) -> bool
					{
						pDest->write((const char*)buff, size);
						return false;
					});
					
					return false;
				});
			}
			catch (gcException &e)
			{
				return e;
			}
			
			info->overridePath(dest.c_str());
			UTIL::FS::delFile(exe.c_str());	
		}
		else
		{
			info->overridePath(exe.c_str());
		}
	}
	else if (args.find("PRECHECK_") == 0)
	{
		info->setInstalled();
	}
	else
	{
		return gcException(ERR_TOOLINSTALL, gcString("Un-supported tool install [{0}]", info->getName()));
	}

	return gcException(ERR_COMPLETED);
}

namespace UserCore
{
namespace Misc
{

ToolInstallThread::ToolInstallThread(ToolManager* toolManager, ::Thread::Mutex &mapLock, std::map<ToolTransactionId, ToolTransInfo*> &transactions) 
	: ::Thread::BaseThread("Tool Install Thread"), m_MapLock(mapLock), m_mTransactions(transactions)
{
	m_CurrentInstall = -1;
	m_pToolManager = toolManager;
	m_bStillInstalling = false;
	
	m_pToolMain = new IPCToolMain();
	m_pToolMain->onCompleteEvent += delegate(this, &ToolInstallThread::onINComplete);
}

ToolInstallThread::~ToolInstallThread()
{
	stop();
	safe_delete(m_pToolMain);
}

IPCToolMain* ToolInstallThread::getToolMain()
{
	return m_pToolMain;
}

bool ToolInstallThread::preInstallStart()
{
	return true;
}

}
}


