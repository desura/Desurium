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

#ifndef DESURA_INSTALLDLPAGE_H
#define DESURA_INSTALLDLPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "InstallBannerPage.h"

#include "usercore/MCFThreadI.h"
#include "usercore/GuiDownloadProvider.h"
#include "mcfcore/DownloadProvider.h"


namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

///////////////////////////////////////////////////////////////////////////////
/// Class InstallDLPage
///////////////////////////////////////////////////////////////////////////////
class InstallDLPage : public InstallBannerPage 
{
public:
	InstallDLPage(wxWindow* parent);
	~InstallDLPage();

	void init();

protected:
	virtual void onButtonPressed(wxCommandEvent& event);

	virtual void onComplete(gcString& );
	virtual void onError(gcException& e);
	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);
	virtual void onPause(bool &state);


private:
	gcButton* m_butPause;
	gcButton* m_butHide;

	bool m_bPaused;
	bool m_bInit;
	bool m_bError;
};

}
}
}

#endif //DESURA_INSTALLDLPAGE_H
