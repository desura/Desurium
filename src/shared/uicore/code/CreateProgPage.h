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

#ifndef DESURA_CreateInfoPage_H
#define DESURA_CreateInfoPage_H
#ifdef _WIN32
#pragma once
#endif

#include "BasePage.h"
#include "wx_controls/gcSpinnerProgBar.h"
#include "mcfcore/ProgressInfo.h"

namespace UserCore
{
	namespace Thread
	{
		class MCFThreadI;
	}
}


///////////////////////////////////////////////////////////////////////////////
/// Class CreateProgPage
///////////////////////////////////////////////////////////////////////////////
class CreateProgPage : public BasePage 
{
public:
	CreateProgPage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
	~CreateProgPage();

	void dispose();

	void setInfo(DesuraId id, const char* path);
	void run();
		
	void setPercent(uint32 per);
	void finished();

protected:
	gcStaticText* m_staticText3;
	wxStaticText* m_labPercent;
	gcSpinnerProgBar* m_pbProgress;
	
	gcButton* m_butPause;
	gcButton* m_butCancel;
	
	void onComplete(gcString& path);
	void onError(gcException& e);
	void onProgress(MCFCore::Misc::ProgressInfo& info);

	void onButtonClick(wxCommandEvent& event);

private:
	bool m_bThreadPaused;
	gcString m_szFolderPath;

	UserCore::Thread::MCFThreadI* m_pThread;
};



#endif //DESURA_CreateInfoPage_H
