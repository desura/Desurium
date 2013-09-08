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

#ifndef DESURA_MWPROGRESSPAGE_H
#define DESURA_MWPROGRESSPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "BasePage.h"
#include "wx_controls/gcControls.h"
#include "wx_controls/gcSpinnerProgBar.h"
#include "wx/wx.h"

#include "usercore/MCFThreadI.h"



class MWProgressPage : public BasePage  
{
public:
	MWProgressPage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL );
	~MWProgressPage();

	void dispose();

	void run();
	void setAddToAccount(bool state){m_bAddToAccount = state;}

	volatile bool isStopped(){return m_bStopped;}

protected:
	gcStaticText* m_labTitle;
	gcStaticText* m_labInfo;
	gcSpinnerProgBar* m_pbProgress;
	
	gcButton* m_butCancel;
	
	void onComplete(uint32&);
	void onError(gcException& e);
	void onProgress(MCFCore::Misc::ProgressInfo& info);

	void onNewItem(gcString& itemName);
	void onItemFound(DesuraId& id);

	void onButtonClicked( wxCommandEvent& event );

	void onClose( wxCloseEvent& event );
private:

	UserCore::Thread::MCFThreadI* m_pThread;
	volatile bool m_bStopped;

	bool m_bAddToAccount;

	DECLARE_EVENT_TABLE()
};


#endif //DESURA_MWPROGRESSPAGE_H
