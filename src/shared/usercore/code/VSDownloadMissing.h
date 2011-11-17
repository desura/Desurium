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

#ifndef DESURA_VSDOWNLOADMISSING_H
#define DESURA_VSDOWNLOADMISSING_H
#ifdef _WIN32
#pragma once
#endif

#include "VSBaseTask.h"
#include "DownloadTask.h"

#include "BDManager.h"

namespace UserCore
{
namespace ItemTask
{

class VSDownloadMissing : public VSBaseTask, public UserCore::Misc::BannerNotifierI
{
public:
	VSDownloadMissing();
	~VSDownloadMissing();

	virtual bool doTask();

	Event<UserCore::Misc::GuiDownloadProvider> onNewProviderEvent;
	virtual void onProgress(MCFCore::Misc::ProgressInfo& prog);

protected:
	virtual void onStop();

	bool checkInstall();
	bool checkComplex();

	void onNewProvider(MCFCore::Misc::DP_s& dp);
	void onBannerComplete(MCFCore::Misc::DownloadProvider &info);

private:
	uint64 m_uiDoneSize;
};

}
}

#endif //DESURA_VSDOWNLOADMISSING_H
