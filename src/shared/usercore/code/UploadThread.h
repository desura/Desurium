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

#ifndef DESURA_MCF_UPLOAD_THREAD_H
#define DESURA_MCF_UPLOAD_THREAD_H
#ifdef _WIN32
#pragma once
#endif


#include "MCFThread.h"
#include "boost/date_time/posix_time/posix_time.hpp"


namespace UserCore
{
namespace Thread
{

class UploadThreadInfo
{
public:
	UploadThreadInfo(DesuraId id, const char* file, const char* key, uint64 start = 0)
	{
		itemId = id;
		szKey = key;
		szFile = file;
		uiStart = start;
	}

	gcString szKey;
	gcString szFile;
	DesuraId itemId;
	uint64 uiStart;
};

class UploadThread : public MCFThread
{
public:
	UploadThread(UploadThreadInfo* info);
	virtual ~UploadThread();

protected:
	void doRun();
	void onProgress(Prog_s& p);

	virtual void onPause();
	virtual void onUnpause();
	virtual void onStop();

	friend class UploadInfoThread;

	EventV onPauseEvent;
	EventV onUnpauseEvent;

private:
	UploadThreadInfo* m_pInfo;
	HttpHandle m_hHttpHandle;

	uint32 m_uiChunkSize;
	uint32 m_uiContinueCount;

	uint64 m_uiFileSize;
	uint64 m_uiAmountRead;

	bool m_bSetPauseStart;
	bool m_bCancel;

	double m_fLastAmmount;

	boost::posix_time::ptime m_tLastTime;
	boost::posix_time::ptime m_tStartTime;
	boost::posix_time::ptime m_tPauseStartTime;
	boost::posix_time::time_duration m_tTotPauseTime;
};


}
}

#endif //DESURA_UploadThread_H
