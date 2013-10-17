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

#ifndef DESURA_MCFTHREAD_H
#define DESURA_MCFTHREAD_H
#ifdef _WIN32
#pragma once
#endif

#include "mcfcore/MCFMain.h"
#include "mcfcore/MCFI.h"


#include "usercore/UploadManagerI.h"
#include "usercore/GuiDownloadProvider.h"
#include "usercore/DownloadInfo.h"
#include "usercore/UploadInfo.h"
#include "usercore/MCFThreadI.h"

#include "util_thread/BaseThread.h"
#include "BaseUserThread.h"

namespace UserCore
{
namespace Thread
{

class MCFThread : public BaseUserThread<MCFThreadI, ::Thread::BaseThread>
{
public:
	MCFThread(const char* threadName, DesuraId id=0, MCFBranch branch = MCFBranch(), MCFBuild build = MCFBuild()) : BaseUserThread<MCFThreadI, ::Thread::BaseThread>(threadName, id)
	{
		m_uiMcfBuild = build;
		m_uiMcfBranch = branch;
		m_pUploadManager = NULL;
	}

	//need to stop here before we delete the mcf
	~MCFThread()
	{
		stop();
	}

	void setUpLoadManager(UserCore::UploadManagerI *um)
	{
		m_pUploadManager = um;
	}

	UserCore::UploadManagerI* getUploadManager()
	{
		return m_pUploadManager;
	}

	Event<MCFCore::Misc::ProgressInfo>* getMcfProgressEvent()
	{
		return &onMcfProgressEvent;
	}

	Event<gcString>* getCompleteStringEvent()
	{
		return &onCompleteStrEvent;
	}

	Event<gcString>* getNewItemEvent()
	{
		return &onNewItemEvent;
	}

	Event<DesuraId>* getItemFoundEvent()
	{
		return &onItemFoundEvent;
	}

	Event<UserCore::Misc::UploadInfo>* getUploadProgressEvent()
	{
		return &onUploadProgressEvent;
	}

	uint32 getMcfBuild()
	{
		return m_uiMcfBuild;
	}

	uint32 getMcfBranch()
	{
		return m_uiMcfBranch;
	}

protected:
	//McfThread
	Event<MCFCore::Misc::ProgressInfo> onMcfProgressEvent;
	Event<gcString> onCompleteStrEvent;

	//modwizard
	Event<gcString> onNewItemEvent;
	Event<DesuraId> onItemFoundEvent;

	//Upload
	Event<UserCore::Misc::UploadInfo> onUploadProgressEvent;

	virtual void onStop()
	{
		if (m_hMCFile.handle())
			m_hMCFile->stop();
	}

	McfHandle m_hMCFile;

private:
	UserCore::UploadManagerI* m_pUploadManager;
	uint32 m_uiMcfBuild;
	uint32 m_uiMcfBranch;
};


}
}

#endif //DESURA_MCFTHREAD_H
