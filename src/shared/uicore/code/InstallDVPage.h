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

#ifndef DESURA_INSTALLDVPAGE_H
#define DESURA_INSTALLDVPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "wx_controls/gcSpinnerProgBar.h"
#include "BaseInstallPage.h"


namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

///////////////////////////////////////////////////////////////////////////////
/// Class InstallGIPage
///////////////////////////////////////////////////////////////////////////////



class InstallDVPage : public BaseInstallPage 
{
public:
	InstallDVPage(wxWindow* parent);
	~InstallDVPage();

	void init();

protected:
	gcStaticText* m_labInfo;
	gcSpinnerProgBar* m_pbProgress;

	gcButton* m_butHide;
	gcButton* m_butCancel;

	void onButtonPressed(wxCommandEvent& event);
	
	void onComplete(uint32& itemId);
	void onError(gcException& e);
	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);

private:
	uint32 m_uiLastFlag;
};

}
}
}


#endif //DESURA_INSTALLDVPAGE_H
