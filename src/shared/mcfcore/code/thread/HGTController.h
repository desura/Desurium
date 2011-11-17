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

#ifndef DESURA_HTTPGETTHREAD_H
#define DESURA_HTTPGETTHREAD_H
#ifdef _WIN32
#pragma once
#endif


#include "BaseMCFThread.h"
#include "WGTExtras.h"

class CourgetteInstance;

namespace MCFCore
{
namespace Thread
{

//! Http get thread controller. Downloads mcf files over http
//!
class HGTController : public MCFCore::Thread::BaseMCFThread
{
public:
	//! Constuctor
	//! 
	//! @param url Url to download from
	//! @param caller Parent Mcf
	//!
	HGTController(const char* url, MCFCore::MCF* caller, const char* insDir);
	~HGTController();

protected:
	void run();

	//! Fills the download list with the files needing to be downloaded
	//!
	//! @param usingDiffs gets set to true when the download will use a diff instead of the actual file
	//!
	void fillDownloadList(bool &usingDiffs);

	//! Does the downloading
	//!
	void doDownload();

	//! Callback from libcurl when data needs to be written
	//!
	//! @param wm Write memory information struct
	//!
	void onWriteMemory(WriteMem_s& wm);

	void onProgress();
	bool saveData(const char* data, uint32 size);

	bool expandDiffs();
	bool expandDiff(CourgetteInstance* ci, MCFCore::MCFFile* file);

	bool writeDiff(uint64 &tot, uint64 fsize, const char* buff, size_t size);

	void decompressDiff(uint64 size, UTIL::FS::FileHandle &fhSrc, UTIL::FS::FileHandle &fhDest);

private:
	gcString m_szUrl;
	gcString m_szInstallDir;

	Misc::WGTSuperBlock* m_pCurBlock;
	std::deque<Misc::WGTSuperBlock*> m_vSuperBlockList;

	uint64 m_uiDownloaded;
	uint64 m_uiTotal;

	UTIL::FS::FileHandle m_hFile;
};

}
}

#endif //DESURA_HTTPGETTHREAD_H
