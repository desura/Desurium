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

#ifndef DESURA_INSTALLINPAGE_H
#define DESURA_INSTALLINPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseInstallPage.h"
#include "wx_controls/gcSpinnerProgBar.h"

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

class InstallINPage : public BaseInstallPage 
{
public:
	InstallINPage(wxWindow* parent);
	~InstallINPage();

protected:
	virtual void init();
	virtual void onComplete(uint32&);
	virtual void onError(gcException& e);
	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);
	virtual void onPause(bool &state);

	void onButtonPressed(wxCommandEvent& event);

	gcStaticText* m_labLabel;
	gcStaticText* m_labInfo;
	gcSpinnerProgBar* m_pbProgress;
	
	gcButton* m_butPause;
	gcButton* m_butHide;

private:
	gcString m_szPath;
	bool m_bPaused;
	bool m_bCompleted;
};

}
}
}

#endif //DESURA_INSTALLINPAGE_H
