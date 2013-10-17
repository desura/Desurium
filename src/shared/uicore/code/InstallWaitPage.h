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

#ifndef DESURA_INSTALLWAITPAGE_H
#define DESURA_INSTALLWAITPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "BaseInstallPage.h"

class ItemPanel;

namespace UI
{
namespace Forms
{
namespace ItemFormPage
{

class InstallWaitPage : public BaseInstallPage 
{
public:
	InstallWaitPage(wxWindow* parent);
	~InstallWaitPage();
	
	void init();

protected:
	wxScrolledWindow* m_swItemList;
	wxFlexGridSizer* m_pSWSizer;

	wxStaticText* m_labInfo;
	
		
	gcButton* m_butCancelAll;
	gcButton* m_butForce;
	gcButton* m_butHide;
	
	virtual void onProgressUpdate(uint32& progress);
	virtual void onMcfProgress(MCFCore::Misc::ProgressInfo& info);

	void onButtonPressed(wxCommandEvent& event);

private:
	std::vector<ItemPanel*> m_vPanelList;

};


}
}
}

#endif //DESURA_InstallWaitPage_H
