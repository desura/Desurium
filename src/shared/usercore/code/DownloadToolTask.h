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
#ifndef DESURA_DOWNLOADTOOLTASK_H
#define DESURA_DOWNLOADTOOLTASK_H
#ifdef _WIN32
#pragma once
#endif

#include "UserTask.h"
#include "usercore/ToolManagerI.h"

namespace UserCore
{
class ToolInfo;
class User;

namespace Task
{

class DownloadToolTask : public UserTask
{
public:
	DownloadToolTask(UserCore::User* user, UserCore::ToolInfo* tool);
	~DownloadToolTask();

	void doTask();

	Event<gcException> onErrorEvent;
	Event<UserCore::Misc::ToolProgress> onProgressEvent;
	EventV onCompleteEvent;

	uint32 getRefCount();
	void increseRefCount();
	void decreaseRefCount(bool forced);

	const char* getName(){return "DownloadToolTask";}

protected:
	void onStop();
	void downloadTool();

	void onProgress(Prog_s& prog);
	void onWrite(WriteMem_s& mem);
	void finish();

private:
	volatile bool m_bStopped;

	ToolInfo* m_pTool;
	HttpHandleI* m_pHttpHandle;

	UTIL::FS::Path m_Path;
	UTIL::FS::FileHandle m_fhFile;

	uint32 m_uiRefCount;
	uint32 m_uiPercent;
};


}
}

#endif //DESURA_DOWNLOADTOOLTASK_H
