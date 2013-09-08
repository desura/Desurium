/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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
#include "CrashDumpThread.h"

#include "managers\CVar.h"
#include "MiniDumpGenerator_Extern.h"

CVar gc_lastusername("gc_lastusername", "");

#ifdef DEBUG
	const char* g_szDesura = "desura-d.exe";
	const char* g_szDesuraService = "desura_service.exe";
#else
	const char* g_szDesura = "desura.exe";
	const char* g_szDesuraService = "desura_service.exe";
#endif

CrashDumpThread::CrashDumpThread() : Thread::BaseThread("CrashDump Thread")
{
	UTIL::FS::Path path(UTIL::OS::getAppDataPath(L"\\dumps\\"), L"", false);
	UTIL::FS::recMakeFolder(path);

	m_szDumpPath = path.getFullPath();

	uploadCrash = NULL;
	uploadCrashProg = NULL;

	m_bSecond = false;
}

CrashDumpThread::~CrashDumpThread()
{
	m_DelMutex.lock();
	m_DelMutex.unlock();
}


bool CrashDumpThread::loadCrashReporter()
{
#ifdef DEBUG
	if (!sol.load("crashuploader-d.dll"))
#else
	if (!sol.load("crashuploader.dll"))
#endif
		return false;

	uploadCrash = sol.getFunction<UploadCrashFn>("UploadCrash");

	if (!uploadCrash)
		return false;

	uploadCrashProg = sol.getFunction<UploadCrashProgFn>("UploadCrashProg");

	if (!uploadCrashProg)
		return false;

	return true;
}

void CrashDumpThread::uploadDump(const char* dumpFile)
{
	if (!uploadCrashProg)
		return;

	int build = 0;
	int branch = 0;

	std::string szAppid = UTIL::WIN::getRegValue(APPID);
	std::string szAppBuild = UTIL::WIN::getRegValue(APPBUILD);

	if (szAppid.size() > 0)
		branch = atoi(szAppid.c_str());

	if (szAppBuild.size() > 0)
		build = atoi(szAppBuild.c_str());

	uploadCrashProg(dumpFile, gc_lastusername.getString(), build, branch, delegate(this, &CrashDumpThread::uploadProgress));
}


void CrashDumpThread::run()
{
	Thread::AutoLock al(m_DelMutex);

	if (!loadCrashReporter())
	{
		gcException e(ERR_NULLHANDLE, "Failed to load utilcore.dll or related function");
		onErrorEvent(e);
		return;
	}

	try
	{
		gcString res = generateDump(g_szDesura);
		uploadDump(res.c_str());
	}
	catch (gcException &e)
	{
		onErrorEvent(e);
		return;
	}

	m_bSecond = true;

	try
	{
		gcString res = generateDump(g_szDesuraService);
		uploadDump(res.c_str());
	}
	catch (gcException &e)
	{
		if (e.getSecErrId() == 0)
		{
			onErrorEvent(e);
			return;
		}
	}

	uint8 prog = 100;
	onProgressEvent(prog);
	onCompleteEvent();
}

void CrashDumpThread::uploadProgress(Prog_s& prog)
{
	uint8 total = 0;

	if (m_bSecond)
		total += 50;

	total += prog.ulnow*50/prog.ultotal;
	onProgressEvent(total);

	prog.abort = isStopped();
}

gcString CrashDumpThread::generateDump(const char* exeName)
{
	MiniDumpGenerator_Extern desuraMDG(exeName, m_szDumpPath.c_str());

	int dRes = desuraMDG.generate();

	if (dRes == -1)
		throw gcException(ERR_NULLHANDLE, dRes, gcString("Failed to find {0}. Is it running?", exeName));
	else if (dRes == 0)
		throw gcException(ERR_NULLHANDLE, dRes, gcString("Failed to produce a dump of {0}.", exeName));

	return gcString("{0}{1}{2}", m_szDumpPath, DIRS_STR, desuraMDG.getSavedFile());
}




