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

#ifndef DESURA_SFWORKERTHREAD_H
#define DESURA_SFWORKERTHREAD_H
#ifdef _WIN32
#pragma once
#endif



class BaseThread;

namespace MCFCore 
{
	class MCFFile;

namespace Thread
{
class SFTController;
class SFTWorkerBuffer;

//! Save file thread worker. Does the work of saving files from mcf to the local system
class SFTWorker : public ::Thread::BaseThread
{
public:
	//! Constructor
	//!
	//! @param controller Parent controller
	//! @param id Worker id
	//!
	SFTWorker(SFTController* controller, uint32 id);
	~SFTWorker();

protected:
	void run();

	//! Gets a new task to perform
	//!
	//! @return True if new task to perform, false if not
	//!
	bool newTask();

	//! Checks to see if the status is a bz2 error
	//!
	//! @return True if bz2 error, false if not
	//!
	bool bzErrorCheck(int32 bzStatus);

	int32 doWork();

	int32 doDecompression(const char* buff, uint32 buffSize, bool endFile);
	int32 doWrite(const char* buff, uint32 buffSize);

	int32 reportError(int32 bz2Code, gcException &e);
	void finishFile();

private:
	uint32 m_uiId;
	SFTController *m_pCT;

	MCFCore::MCFFile *m_pCurFile;

	UTIL::MISC::BZ2Worker *m_pBzs;
	UTIL::FS::FileHandle m_hFh;
};

}
}

#endif
