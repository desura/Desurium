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

#ifndef DESURA_INSTALLVIPAGE_H
#define DESURA_INSTALLVIPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "usercore/ItemInfoI.h"
#include "usercore/VerifyComplete.h"
#include "InstallBannerPage.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

///////////////////////////////////////////////////////////////////////////////
/// Class InstallVIPage
///////////////////////////////////////////////////////////////////////////////
class InstallVIPage : public InstallBannerPage 
{
public:
	InstallVIPage(wxWindow* parent);
	~InstallVIPage();

	void init();

protected:
	virtual void onButtonPressed(wxCommandEvent& event);

	virtual void onError(gcException& e);
	virtual void onVerifyComplete(UserCore::Misc::VerifyComplete& info);
	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);

private:
	uint32 m_iStage;
};


}
}
}

#endif //DESURA_INSTALLVIPAGE_H
