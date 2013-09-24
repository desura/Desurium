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
#include "UtilFunction.h"

#include "usercore/UserCoreI.h"
#include "usercore/UserThreadManagerI.h"
#include "usercore/MCFThreadI.h"
#include "util_thread/BaseThread.h"

#include "SharedObjectLoader.h"


class UploadMCF : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 5;
	}

	virtual const char* getArgDesc(size_t index)
	{
		switch (index)
		{
		default:
		case 0:
			return "Username";

		case 1:
			return "Password";

		case 2:
			return "Folder";

		case 3:
			return "Id";

		case 4:
			return "Area (mods|games)";
		}
	}

	virtual const char* getFullArg()
	{
		return "upload";
	}

	virtual const char getShortArg()
	{
		return 'l';
	}

	virtual const char* getDescription()
	{
		return "Uploads a MCF to desura";
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		DesuraId id(args[3].c_str(), args[4].c_str());

		if (!Login(args[0], args[1]))
			return -1;

		if (!CreateMcf(id, args[2]))
			return -2;

		if (!StartUpload(id))
			return -3;

		if (!UploadMcf())
			return -4;

		return 0;
	}


	void OnNeedCvar(UserCore::Misc::CVar_s &cvar)
	{

	}

	typedef void* (*FactoryBuilderUCfn)(const char* name);

	SharedObjectLoader m_hUserCore;

	bool Login(std::string &strUsername, std::string &strPassword)
	{
		gcString path = UTIL::OS::getAppDataPath();

#ifdef WIN32
		const char* szDllName = "usercore.dll";
#else
		const char* szDllName = "libusercore.so";
#endif

		if (!m_hUserCore.load(szDllName))
		{
			Msg(gcString("Failed to load {0}", szDllName).c_str());
			return false;
		}

		FactoryBuilderUCfn usercoreFactory = m_hUserCore.getFunction<FactoryBuilderUCfn>("FactoryBuilderUC");

		if (!usercoreFactory)
		{
			Msg("Failed to find factory builder function");
			return false;
		}

		g_pUserHandle = (UserCore::UserI*)usercoreFactory(USERCORE);
		g_pUserHandle->init(path.c_str());

		try
		{
			//need to do this here as news items will be passed onlogin
			*g_pUserHandle->getNeedCvarEvent() += delegate(this, &UploadMCF::OnNeedCvar);

			g_pUserHandle->lockDelete();
			g_pUserHandle->logInTool(strUsername.c_str(), strPassword.c_str());
			g_pUserHandle->unlockDelete();
		}
		catch (gcException e)
		{
			g_pUserHandle->logOut();
			g_pUserHandle->unlockDelete();
			safe_delete(g_pUserHandle);

			Warning(gcString("Failed to login: {0}", e));
		}

		return !!g_pUserHandle;
	}

	void OnMCFCreateError(gcException &e)
	{
		Warning(gcString("Failed to create mcf: {0}", e));
		g_WaitCon.notify();
	}

	void OnMCFCreateComplete(gcString &strPath)
	{
		Msg("Completed creating Mcf.\n");
		g_strMcfOutPath = strPath;
		g_WaitCon.notify();
	}

	bool CreateMcf(DesuraId id, std::string &strFolderPath)
	{
		Msg("Creating Mcf....\n");

		UserCore::Thread::MCFThreadI* pThread = g_pUserHandle->getThreadManager()->newCreateMCFThread(id, strFolderPath.c_str());

		*pThread->getMcfProgressEvent() += delegate((UtilFunction*)this, &UtilFunction::printProgress);
		*pThread->getErrorEvent() += delegate(this, &UploadMCF::OnMCFCreateError);
		*pThread->getCompleteStringEvent() += delegate(this, &UploadMCF::OnMCFCreateComplete);

		pThread->start();

		g_WaitCon.wait();
		safe_delete(pThread);

		return g_strMcfOutPath.size() != 0;
	}

	void OnStartUploadError(gcException &e)
	{
		Warning(gcString("Failed to init upload of mcf: {0}", e));
		g_WaitCon.notify();
	}

	void OnStartUploadComplete(gcString &strHash)
	{
		g_strUploadHash = strHash;
		g_WaitCon.notify();
	}

	bool StartUpload(DesuraId id)
	{
		Msg("Starting Mcf Upload....\n");

		//start upload
		UserCore::Thread::MCFThreadI* pPrepThread = g_pUserHandle->getThreadManager()->newUploadPrepThread(id, g_strMcfOutPath.c_str());

		*pPrepThread->getErrorEvent() += delegate(this, &UploadMCF::OnStartUploadError);
		*pPrepThread->getCompleteStringEvent() += delegate(this, &UploadMCF::OnStartUploadComplete);

		pPrepThread->start();

		g_WaitCon.wait();

		return g_strUploadHash.size() != 0;
	}


	void OnUploadProgress(UserCore::Misc::UploadInfo &info)
	{
		static uint32 s_uiLastProg = -1;
		if (s_uiLastProg == info.percent)
			return;

		s_uiLastProg = info.percent;

		printf("\r[");

		size_t tot = (info.percent / 2);

		for (size_t x = 0; x < 50; x++)
		{
			if (x < tot)
				printf("=");
			else
				printf(" ");
		}

		std::string done = UTIL::MISC::niceSizeStr(info.doneAmmount, true);
		std::string total = UTIL::MISC::niceSizeStr(info.totalAmmount);
		std::string lab = UTIL::MISC::genTimeString(info.hour, info.min, info.rate);

		printf("] %u%% [done %s of %s] ETA: %s", info.percent, done.c_str(), total.c_str(), lab.c_str());
	}

	void OnUploadError(gcException &e)
	{
		Warning(gcString("Failed to upload of mcf: {0}", e));
		g_WaitCon.notify();
	}

	void OnUploadComplete(uint32& status)
	{
		Msg("Upload complete\n");
		g_bUploadDone = true;
		g_WaitCon.notify();
	}

	bool UploadMcf()
	{
		Msg("Uploading Mcf....\n");

		//upload
		UserCore::Misc::UploadInfoThreadI* info = g_pUserHandle->getUploadManager()->findItem(g_strUploadHash.c_str());
		assert(info);

		*info->getUploadProgressEvent() += delegate(this, &UploadMCF::OnUploadProgress);
		*info->getErrorEvent() += delegate(this, &UploadMCF::OnUploadError);
		*info->getCompleteEvent() += delegate(this, &UploadMCF::OnUploadComplete);

		if (info->isPaused())
			info->unpause();

		info->start();
		g_WaitCon.wait();

		return g_bUploadDone;
	}

private:
	UserCore::UserI* g_pUserHandle;
	Thread::WaitCondition g_WaitCon;
	gcString g_strMcfOutPath;
	gcString g_strUploadHash;
	bool g_bUploadDone;
};

REG_FUNCTION(UploadMCF)