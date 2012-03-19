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

#include "usercore\UserCoreI.h"
#include "usercore\UserThreadManagerI.h"
#include "usercore\MCFThreadI.h"

#include "util_thread\BaseThread.h"

UserCore::UserI* g_pUserHandle;

Thread::WaitCondition g_WaitCon;
gcString g_strMcfOutPath;
gcString g_strUploadHash;
bool g_bUploadDone;

bool Login(std::string &strUsername, std::string &strPassword);
bool CreateMcf(DesuraId id, std::string &strFolderPath);
bool StartUpload(DesuraId id);
bool UploadMcf();

void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);

#ifdef DEBUG
	char out[1024]={0};
	vsnprintf(out, 1024, format, args);
	OutputDebugString(out);
#endif

	va_end(args);
}

class Color;

void LogMsg(int type, std::string msg, Color *col)
{
	printf("%s", msg.c_str());
}

void LogMsg(int type, std::wstring msg, Color *col)
{
	wprintf(L"%s", msg.c_str());
}

void ShowHelp()
{
	printf("----------------------------------------------------------------------------\n");
	printf(" MCF Upload Utility By Lodle\n");
	printf("----------------------------------------------------------------------------\n");
	printf("\n");
	printf("Usage:\n");
	printf("\t--username (-u): Desura Username\n");
	printf("\t--password (-p): Desura Password\n");
	printf("\t--folder (-f): Folder of game files\n");
	printf("\t--item (-i): Item site area id\n");
	printf("\t--type (-t): Item site area (mods | games)\n");
}

std::map<std::string, std::string> ProcessArgs(int argc, char** argv)
{
	std::map<std::string, std::string> args;

	for (int x=1; x<argc; x++)
	{
		if (!argv[x] || argv[x][0] != '-')
			continue;

		if ((x+1) > argc && argv[x][0] == '-')
		{
			args[argv[x]] = argv[x+1];
			x++;
		}
		else
		{
			args[argv[x]] = "";
		}
	}

	std::for_each(args.begin(), args.end(), [&args](std::pair<std::string, std::string> p)
	{
		std::string strKey = p.first;

		if (strKey.size() >= 3 && strKey[0] == '-' && strKey[1] == '-')
		{
			std::string strSort("-");
			strSort += strKey[2];
			args[strSort] = p.second;
		}
	});

	return args;
}

bool ValidateArgs(std::map<std::string, std::string> &args)
{
	if (args["-u"].size() == 0)
		return false;

	if (args["-p"].size() == 0)
		return false;

	if (args["-f"].size() == 0)
		return false;

	if (args["-i"].size() == 0)
		return false;

	if (args["-t"].size() == 0)
		return false;
	
	return true;
}

int main(int argc, char** argv)
{
	std::map<std::string, std::string> args = ProcessArgs(argc, argv);

	if (args.size() != 5 || !ValidateArgs(args))
	{
		ShowHelp();
		return -5;
	}

	DesuraId id(args["-i"].c_str(), args["-t"].c_str());

	if (!Login(args["-u"], args["-p"]))
		return -1;

	if (!CreateMcf(id, args["-f"]))
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

bool Login(std::string &strUsername, std::string &strPassword)
{
	gcString path = UTIL::OS::getAppDataPath();

	g_pUserHandle = (UserCore::UserI*)UserCore::FactoryBuilderUC(USERCORE);
	g_pUserHandle->init(path.c_str());

	try
	{
		//need to do this here as news items will be passed onlogin
		*g_pUserHandle->getNeedCvarEvent() += delegate(&OnNeedCvar);

		g_pUserHandle->lockDelete();
		g_pUserHandle->logIn(strUsername.c_str(), strPassword.c_str());
		g_pUserHandle->unlockDelete();
	}
	catch (gcException e)
	{
		g_pUserHandle->logOut();
		g_pUserHandle->unlockDelete();
		safe_delete(g_pUserHandle);

		Warning(gcString("Failed to login: {0}", e));
	}

	return !g_pUserHandle;
}




void OnMCFCreateProgress(MCFCore::Misc::ProgressInfo &prog)
{
	static uint32 s_uiLastProg = -1;
	if (s_uiLastProg == prog.percent)
		return;

	s_uiLastProg = prog.percent;

	printf("\r[");

	size_t tot = (prog.percent/2);

	for (size_t x=0; x<50; x++)
	{
		if (x<tot)
			printf("=");
		else
			printf(" ");
	}

	printf("] %u%%", prog.percent);
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

	*pThread->getMcfProgressEvent() += delegate(&OnMCFCreateProgress);
	*pThread->getErrorEvent() += delegate(&OnMCFCreateError);
	*pThread->getCompleteStringEvent() += delegate(&OnMCFCreateComplete);

	pThread->start();
	safe_delete(pThread);

	g_WaitCon.wait();

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

	*pPrepThread->getErrorEvent() += delegate(&OnStartUploadError);
	*pPrepThread->getCompleteStringEvent() += delegate(&OnStartUploadComplete);

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

	size_t tot = (info.percent/2);

	for (size_t x=0; x<50; x++)
	{
		if (x<tot)
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

	*info->getUploadProgressEvent() += delegate(&OnUploadProgress);
	*info->getErrorEvent() += delegate(&OnUploadError);
	*info->getCompleteEvent() += delegate(&OnUploadComplete);

	if (info->isPaused())
		info->unpause();

	info->start();
	g_WaitCon.wait();

	return g_bUploadDone;
}